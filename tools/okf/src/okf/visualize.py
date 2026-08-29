"""Self-contained force-directed graph viewer for a knowledge bundle."""

from __future__ import annotations

import json
from pathlib import Path

from .bundle import load_bundle, md_links, resolve_link

HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<title>Knowledge viz</title>
<style>
  html,body{margin:0;height:100%;font:14px/1.4 system-ui,sans-serif;background:#111;color:#eee}
  #wrap{display:grid;grid-template-columns:1fr 320px;height:100%}
  canvas{width:100%;height:100%;display:block;background:#0b0b0b}
  aside{border-left:1px solid #333;padding:12px;overflow:auto}
  input,select{width:100%;margin:4px 0 12px;padding:6px;background:#222;border:1px solid #444;color:#eee}
  .muted{color:#888;font-size:12px}
  a{color:#8cf}
</style>
</head>
<body>
<div id="wrap">
  <canvas id="c"></canvas>
  <aside>
    <label>Search</label>
    <input id="q" placeholder="filter…"/>
    <label>Type</label>
    <select id="type"><option value="">(all)</option></select>
    <div id="detail"><p class="muted">Click a node</p></div>
  </aside>
</div>
<script>
const DATA = __DATA__;
const nodes = DATA.nodes.map(n => ({...n, x: Math.random()*800, y: Math.random()*600, vx:0, vy:0}));
const byId = Object.fromEntries(nodes.map(n => [n.id, n]));
const links = DATA.links.map(l => ({source: byId[l.source], target: byId[l.target]})).filter(l => l.source && l.target);
const canvas = document.getElementById('c');
const ctx = canvas.getContext('2d');
const detail = document.getElementById('detail');
const qEl = document.getElementById('q');
const typeEl = document.getElementById('type');
[...new Set(nodes.map(n => n.type))].sort().forEach(t => {
  const o = document.createElement('option'); o.value = t; o.textContent = t; typeEl.appendChild(o);
});
function resize(){ canvas.width = canvas.clientWidth; canvas.height = canvas.clientHeight; }
resize(); addEventListener('resize', resize);
let selected = null;
function visible(n){
  const q = qEl.value.toLowerCase();
  const t = typeEl.value;
  if (t && n.type !== t) return false;
  if (q && !(n.title.toLowerCase().includes(q) || n.id.toLowerCase().includes(q))) return false;
  return true;
}
function tick(){
  const w = canvas.width, h = canvas.height;
  for (const n of nodes){ n.vx *= 0.85; n.vy *= 0.85; }
  for (let i=0;i<nodes.length;i++) for (let j=i+1;j<nodes.length;j++){
    const a=nodes[i], b=nodes[j];
    let dx=a.x-b.x, dy=a.y-b.y; let d=Math.hypot(dx,dy)||1;
    const f=40/(d*d); a.vx+=dx/d*f; a.vy+=dy/d*f; b.vx-=dx/d*f; b.vy-=dy/d*f;
  }
  for (const l of links){
    let dx=l.target.x-l.source.x, dy=l.target.y-l.source.y; let d=Math.hypot(dx,dy)||1;
    const f=(d-120)*0.01; l.source.vx+=dx/d*f; l.source.vy+=dy/d*f; l.target.vx-=dx/d*f; l.target.vy-=dy/d*f;
  }
  for (const n of nodes){ n.vx+=(w/2-n.x)*0.002; n.vy+=(h/2-n.y)*0.002; n.x+=n.vx; n.y+=n.vy; }
  ctx.clearRect(0,0,w,h);
  ctx.strokeStyle='#333';
  for (const l of links){
    if (!visible(l.source)||!visible(l.target)) continue;
    ctx.beginPath(); ctx.moveTo(l.source.x,l.source.y); ctx.lineTo(l.target.x,l.target.y); ctx.stroke();
  }
  for (const n of nodes){
    if (!visible(n)) continue;
    ctx.fillStyle = n===selected ? '#fc6' : '#4af';
    ctx.beginPath(); ctx.arc(n.x,n.y,6,0,Math.PI*2); ctx.fill();
    ctx.fillStyle='#ccc'; ctx.fillText(n.title, n.x+8, n.y+4);
  }
  requestAnimationFrame(tick);
}
canvas.addEventListener('click', e => {
  const r = canvas.getBoundingClientRect();
  const x = e.clientX-r.left, y = e.clientY-r.top;
  selected = null;
  for (const n of nodes){ if (visible(n) && Math.hypot(n.x-x,n.y-y)<10){ selected=n; break; } }
  if (!selected){ detail.innerHTML='<p class="muted">Click a node</p>'; return; }
  const backs = links.filter(l => l.target===selected).map(l => l.source);
  detail.innerHTML = `<h2>${selected.title}</h2>
    <p class="muted">${selected.type} · ${selected.id}</p>
    <p>${selected.description||''}</p>
    <p><strong>Backlinks</strong></p>
    <ul>${backs.map(b=>`<li>${b.title}</li>`).join('')||'<li class="muted">none</li>'}</ul>`;
});
qEl.oninput = typeEl.onchange = () => {};
tick();
</script>
</body>
</html>
"""


def visualize(root: Path, out: Path) -> Path:
    bundle = load_bundle(root)
    nodes = []
    links = []
    for c in bundle.concepts:
        nodes.append(
            {
                "id": c.rel,
                "title": c.meta.get("title") or Path(c.rel).stem,
                "type": c.meta.get("type") or "",
                "description": c.meta.get("description") or "",
            }
        )
        full = c.body + ("\n" + c.citations if c.citations else "")
        for _a, t in md_links(full):
            hit = resolve_link(bundle, c.rel, t)
            if not hit or hit.suffix != ".md" or hit.name in {"index.md", "log.md"}:
                continue
            try:
                target = hit.resolve().relative_to(bundle.root.resolve()).as_posix()
            except ValueError:
                continue
            links.append({"source": c.rel, "target": target})
    data = json.dumps({"nodes": nodes, "links": links})
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(HTML_TEMPLATE.replace("__DATA__", data), encoding="utf-8")
    return out
