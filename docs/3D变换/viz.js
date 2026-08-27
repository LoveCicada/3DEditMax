/**
 * Canvas 线框可视化：用正在讲解的矩阵把 3D 投到屏幕上。
 */
(function (root) {
  "use strict";

  const C = {
    x: "#ef476f",
    y: "#06d6a0",
    z: "#4cc9f0",
    world: "#4cc9f0",
    view: "#b388ff",
    proj: "#ffb703",
    mvp: "#f72585",
    model: "#7be7c5",
    grid: "rgba(255,255,255,0.07)",
    line: "rgba(238,241,249,0.55)",
    dim: "rgba(238,241,249,0.22)",
    text: "#b3bbd1",
    title: "#eef1f9"
  };

  const CUBE_EDGES = [
    [0, 1], [1, 2], [2, 3], [3, 0],
    [4, 5], [5, 6], [6, 7], [7, 4],
    [0, 4], [1, 5], [2, 6], [3, 7]
  ];

  function cubeCorners(s) {
    const h = s * 0.5;
    return [
      [-h, -h, -h], [h, -h, -h], [h, h, -h], [-h, h, -h],
      [-h, -h, h], [h, -h, h], [h, h, h], [-h, h, h]
    ];
  }

  function fitCanvas(canvas) {
    const dpr = window.devicePixelRatio || 1;
    const parent = canvas.parentElement;
    const w = Math.max(1, Math.floor(parent.clientWidth));
    const h = Math.max(1, Math.floor(parent.clientHeight));
    const bw = Math.floor(w * dpr);
    const bh = Math.floor(h * dpr);
    if (canvas.width !== bw || canvas.height !== bh) {
      canvas.width = bw;
      canvas.height = bh;
    }
    canvas.style.width = w + "px";
    canvas.style.height = h + "px";
    const ctx = canvas.getContext("2d");
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    return { ctx: ctx, w: w, h: h };
  }

  function diagramView(w, h, eye, at) {
    const aspect = w / Math.max(1, h);
    const view = DX.lookAtLH(eye, at, [0, 1, 0]);
    const proj = DX.perspectiveFovLH(DX.deg(42), aspect, 0.2, 80);
    return { view: view, proj: proj };
  }

  function orbitDiagram(w, h, dist, pitch, yaw, target) {
    const eye = DX.orbitEye(dist, pitch, yaw, target || [0, 0.4, 0]);
    return diagramView(w, h, eye, target || [0, 0.4, 0]);
  }

  function begin3D(ctx, w, h, view, proj, ox, oy, vw, vh) {
    ox = ox || 0;
    oy = oy || 0;
    vw = vw || w;
    vh = vh || h;
    const lines = [];
    const dots = [];
    const labels = [];

    function toScreen(p) {
      const s = DX.projectToScreen(p, view, proj, vw, vh);
      if (!s) {
        return null;
      }
      return { x: s.x + ox, y: s.y + oy, z: s.z };
    }

    function line(a, b, color, width, alpha) {
      const sa = toScreen(a);
      const sb = toScreen(b);
      if (!sa || !sb) {
        return;
      }
      if (sa.z < -0.2 || sa.z > 1.4 || sb.z < -0.2 || sb.z > 1.4) {
        return;
      }
      lines.push({
        a: sa,
        b: sb,
        color: color,
        width: width || 1.4,
        alpha: alpha === undefined ? 1 : alpha,
        z: (sa.z + sb.z) * 0.5
      });
    }

    function point(p, color, r) {
      const s = toScreen(p);
      if (!s || s.z < -0.2 || s.z > 1.4) {
        return s;
      }
      dots.push({ x: s.x, y: s.y, z: s.z, color: color, r: r || 5 });
      return s;
    }

    function label(p, text, color, dy) {
      const s = toScreen(p);
      if (!s || s.z < -0.2 || s.z > 1.4) {
        return;
      }
      labels.push({ x: s.x, y: s.y + (dy || -10), z: s.z, text: text, color: color || C.text });
    }

    function axes(origin, len, matrix) {
      const o = origin || [0, 0, 0];
      const L = len || 1.4;
      const ends = [[L, 0, 0], [0, L, 0], [0, 0, L]];
      const cols = [C.x, C.y, C.z];
      const names = ["X", "Y", "Z"];
      for (let i = 0; i < 3; i++) {
        let a = o;
        let b = DX.add3(o, ends[i]);
        if (matrix) {
          a = DX.transformCoord(o, matrix).slice(0, 3);
          b = DX.transformCoord(DX.add3(o, ends[i]), matrix).slice(0, 3);
        }
        line(a, b, cols[i], 2.2);
        label(b, names[i], cols[i], -6);
      }
    }

    function grid(n, step) {
      n = n || 4;
      step = step || 1;
      const lim = n * step;
      for (let i = -n; i <= n; i++) {
        const d = i * step;
        line([-lim, 0, d], [lim, 0, d], C.grid, 1, 1);
        line([d, 0, -lim], [d, 0, lim], C.grid, 1, 1);
      }
    }

    function cube(matrix, color, size) {
      const corners = cubeCorners(size || 1);
      const pts = corners.map(function (p) {
        return matrix ? DX.transformCoord(p, matrix).slice(0, 3) : p;
      });
      for (let i = 0; i < CUBE_EDGES.length; i++) {
        const e = CUBE_EDGES[i];
        line(pts[e[0]], pts[e[1]], color || C.line, 1.6);
      }
      return pts;
    }

    function flush() {
      lines.sort(function (a, b) { return b.z - a.z; });
      for (let i = 0; i < lines.length; i++) {
        const L = lines[i];
        ctx.globalAlpha = L.alpha;
        ctx.strokeStyle = L.color;
        ctx.lineWidth = L.width;
        ctx.lineCap = "round";
        ctx.beginPath();
        ctx.moveTo(L.a.x, L.a.y);
        ctx.lineTo(L.b.x, L.b.y);
        ctx.stroke();
      }
      ctx.globalAlpha = 1;
      dots.sort(function (a, b) { return b.z - a.z; });
      for (let i = 0; i < dots.length; i++) {
        const d = dots[i];
        ctx.beginPath();
        ctx.fillStyle = d.color;
        ctx.shadowColor = d.color;
        ctx.shadowBlur = 12;
        ctx.arc(d.x, d.y, d.r, 0, Math.PI * 2);
        ctx.fill();
        ctx.shadowBlur = 0;
      }
      ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";
      ctx.textAlign = "center";
      for (let i = 0; i < labels.length; i++) {
        const lb = labels[i];
        ctx.fillStyle = lb.color;
        ctx.fillText(lb.text, lb.x, lb.y);
      }
    }

    return {
      toScreen: toScreen,
      line: line,
      point: point,
      label: label,
      axes: axes,
      grid: grid,
      cube: cube,
      flush: flush
    };
  }

  function clear(ctx, w, h) {
    ctx.clearRect(0, 0, w, h);
  }

  function pane(ctx, x, y, w, h, title) {
    ctx.save();
    ctx.beginPath();
    ctx.rect(x, y, w, h);
    ctx.clip();
    ctx.fillStyle = "rgba(8,12,22,0.35)";
    ctx.fillRect(x, y, w, h);
    ctx.fillStyle = C.title;
    ctx.font = "600 12px Segoe UI, Microsoft YaHei, sans-serif";
    ctx.textAlign = "left";
    ctx.fillText(title, x + 12, y + 20);
    return function end() {
      ctx.restore();
      ctx.strokeStyle = "rgba(39,46,68,0.9)";
      ctx.lineWidth = 1;
      ctx.strokeRect(x + 0.5, y + 0.5, w - 1, h - 1);
    };
  }

  function roundRect(ctx, x, y, w, h, r) {
    ctx.beginPath();
    ctx.moveTo(x + r, y);
    ctx.arcTo(x + w, y, x + w, y + h, r);
    ctx.arcTo(x + w, y + h, x, y + h, r);
    ctx.arcTo(x, y + h, x, y, r);
    ctx.arcTo(x, y, x + w, y, r);
    ctx.closePath();
  }

  function drawPipeline(ctx, w, h, t) {
    clear(ctx, w, h);
    const stations = [
      { name: "模型空间", sub: "顶点出生地", color: C.model },
      { name: "世界空间", sub: "× 世界矩阵", color: C.world },
      { name: "视图空间", sub: "× 视图矩阵", color: C.view },
      { name: "裁剪 / NDC", sub: "× 投影矩阵", color: C.proj },
      { name: "屏幕像素", sub: "视口映射", color: C.mvp }
    ];
    const n = stations.length;
    const boxW = Math.min(150, (w - 48) / n - 12);
    const boxH = 72;
    const y = h * 0.42;
    const span = w - 56 - boxW;
    const x0 = 28;

    ctx.strokeStyle = "rgba(91,141,239,0.35)";
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(x0 + boxW * 0.5, y + boxH * 0.5);
    ctx.lineTo(x0 + span + boxW * 0.5, y + boxH * 0.5);
    ctx.stroke();

    const pos = [];
    for (let i = 0; i < n; i++) {
      const x = x0 + (span * i) / (n - 1);
      pos.push({ x: x + boxW * 0.5, y: y + boxH * 0.5, boxX: x });
      const on = Math.abs(t * (n - 1) - i) < 0.55;
      ctx.save();
      if (on) {
        ctx.shadowColor = stations[i].color;
        ctx.shadowBlur = 18;
      }
      roundRect(ctx, x, y, boxW, boxH, 10);
      ctx.fillStyle = on ? "rgba(22,27,43,0.95)" : "rgba(14,19,32,0.92)";
      ctx.fill();
      ctx.shadowBlur = 0;
      ctx.strokeStyle = stations[i].color;
      ctx.lineWidth = on ? 2 : 1;
      ctx.stroke();
      ctx.restore();
      ctx.fillStyle = stations[i].color;
      ctx.font = "600 13px Segoe UI, Microsoft YaHei, sans-serif";
      ctx.textAlign = "center";
      ctx.fillText(stations[i].name, x + boxW / 2, y + 30);
      ctx.fillStyle = C.text;
      ctx.font = "11px Segoe UI, Microsoft YaHei, sans-serif";
      ctx.fillText(stations[i].sub, x + boxW / 2, y + 50);
    }

    const u = t * (n - 1);
    const i0 = Math.min(n - 2, Math.floor(u));
    const f = u - i0;
    const px = pos[i0].x + (pos[i0 + 1].x - pos[i0].x) * f;
    const py = pos[i0].y + Math.sin(f * Math.PI) * -28;
    ctx.beginPath();
    ctx.fillStyle = C.mvp;
    ctx.shadowColor = C.mvp;
    ctx.shadowBlur = 16;
    ctx.arc(px, py, 8, 0, Math.PI * 2);
    ctx.fill();
    ctx.shadowBlur = 0;
    ctx.fillStyle = C.title;
    ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";
    ctx.textAlign = "center";
    ctx.fillText("一个顶点", px, py - 16);

    ctx.fillStyle = C.text;
    ctx.font = "13px Segoe UI, Microsoft YaHei, sans-serif";
    ctx.fillText("GPU 对每个顶点做同一件事：连续乘上几张「搬家说明书」。", w / 2, h - 28);
  }

  function drawTrack(ctx, w, h, state) {
    clear(ctx, w, h);
    const cam = orbitDiagram(w, h, 6.2, 22, 38 + (state.playT || 0) * 8, [0.3, 0.3, 0.3]);
    const g = begin3D(ctx, w, h, cam.view, cam.proj);
    g.grid(3, 1);
    g.axes([0, 0, 0], 1.6, null);
    const W = DX.composeWorld(
      state.sx, state.sy, state.sz,
      DX.deg(state.pitch), DX.deg(state.yaw), DX.deg(state.roll),
      state.tx, state.ty, state.tz
    );
    g.cube(DX.identity(), C.dim, 1);
    g.cube(W, C.world, 1);
    g.axes([0, 0, 0], 0.7, W);
    const p = [state.px, state.py, state.pz];
    const pw = DX.transformCoord(p, W).slice(0, 3);
    g.point(p, C.model, 4);
    g.label(p, "模型点", C.model, -14);
    g.point(pw, C.mvp, 6);
    g.label(pw, "世界位置", C.mvp, -14);
    g.line(p, pw, "rgba(247,37,133,0.35)", 1.2);
    g.flush();
  }

  function drawTrs(ctx, w, h, state) {
    clear(ctx, w, h);
    const cam = orbitDiagram(w, h, 7, 18, 42, [0, 0.2, 0]);
    const g = begin3D(ctx, w, h, cam.view, cam.proj);
    g.grid(4, 1);
    g.axes([0, 0, 0], 2.0, null);
    const W = DX.composeWorld(
      state.sx, state.sy, state.sz,
      DX.deg(state.pitch), DX.deg(state.yaw), DX.deg(state.roll),
      state.tx, state.ty, state.tz
    );
    g.cube(DX.identity(), C.dim, 1);
    g.cube(W, C.world, 1);
    g.axes([0, 0, 0], 0.85, W);
    g.flush();
    ctx.fillStyle = C.text;
    ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";
    ctx.textAlign = "left";
    ctx.fillText("淡色 = 模型空间原位　亮青 = 乘上世界矩阵之后", 14, h - 14);
  }

  function planetMatrix(spin) {
    return DX.composeWorld(1, 1, 1, 0, DX.deg(spin), 0, 0, 0.6, 0);
  }

  function satLocalMatrix(orbit, spin) {
    return DX.mulMany(
      DX.scaling(0.35, 0.35, 0.35),
      DX.rotationY(DX.deg(spin * 3)),
      DX.translation(1.8, 0, 0),
      DX.rotationY(DX.deg(orbit))
    );
  }

  function drawParent(ctx, w, h, state) {
    clear(ctx, w, h);
    const cam = orbitDiagram(w, h, 8.2, 24, 28, [0, 0.5, 0]);
    const g = begin3D(ctx, w, h, cam.view, cam.proj);
    g.grid(4, 1);
    g.axes([0, 0, 0], 1.8, null);
    const planet = planetMatrix(state.spin);
    const local = satLocalMatrix(state.orbit, state.spin);
    const satWorld = DX.mul(local, planet);
    g.cube(planet, C.world, 1.2);
    g.axes([0, 0, 0], 0.9, planet);
    g.cube(satWorld, C.mvp, 1);
    g.axes([0, 0, 0], 0.55, satWorld);
    const orbitPts = [];
    for (let i = 0; i <= 48; i++) {
      const a = (i / 48) * Math.PI * 2;
      const loc = satLocalMatrix(DX.radToDeg(a), 0);
      orbitPts.push(DX.transformCoord([0, 0, 0], DX.mul(loc, planet)).slice(0, 3));
    }
    for (let i = 0; i < orbitPts.length - 1; i++) {
      g.line(orbitPts[i], orbitPts[i + 1], "rgba(247,37,133,0.35)", 1.2);
    }
    g.flush();
    ctx.fillStyle = C.world;
    ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";
    ctx.textAlign = "left";
    ctx.fillText("行星世界矩阵", 14, 22);
    ctx.fillStyle = C.mvp;
    ctx.fillText("卫星世界 = 卫星局部 × 行星世界", 14, 40);
  }

  function cameraGizmo(g, eye, at) {
    const fwd = DX.normalize3(DX.sub3(at, eye));
    const right = DX.normalize3(DX.cross3([0, 1, 0], fwd));
    const up = DX.cross3(fwd, right);
    g.point(eye, C.view, 5);
    g.line(eye, DX.add3(eye, DX.scale3(right, 0.55)), C.x, 2);
    g.line(eye, DX.add3(eye, DX.scale3(up, 0.55)), C.y, 2);
    g.line(eye, DX.add3(eye, DX.scale3(fwd, 0.9)), C.z, 2);
    const near = 0.55;
    const far = 1.85;
    const hw = 0.4;
    const hh = 0.28;
    const corners = [];
    const depths = [near, far];
    const scales = [1, 2.1];
    for (let k = 0; k < 2; k++) {
      const s = scales[k];
      const d = depths[k];
      const c = [
        DX.add3(eye, DX.add3(DX.scale3(fwd, d), DX.add3(DX.scale3(right, -hw * s), DX.scale3(up, -hh * s)))),
        DX.add3(eye, DX.add3(DX.scale3(fwd, d), DX.add3(DX.scale3(right, hw * s), DX.scale3(up, -hh * s)))),
        DX.add3(eye, DX.add3(DX.scale3(fwd, d), DX.add3(DX.scale3(right, hw * s), DX.scale3(up, hh * s)))),
        DX.add3(eye, DX.add3(DX.scale3(fwd, d), DX.add3(DX.scale3(right, -hw * s), DX.scale3(up, hh * s))))
      ];
      corners.push(c);
    }
    for (let i = 0; i < 4; i++) {
      g.line(corners[0][i], corners[0][(i + 1) % 4], C.view, 1.3);
      g.line(corners[1][i], corners[1][(i + 1) % 4], C.view, 1.3);
      g.line(corners[0][i], corners[1][i], C.view, 1.1);
      g.line(eye, corners[0][i], "rgba(179,136,255,0.45)", 1);
    }
    g.label(eye, "相机", C.view, -14);
  }

  function drawCamera(ctx, w, h, state) {
    clear(ctx, w, h);
    const mid = w * 0.5;
    const target = [0, 0.4, 0];
    const eye = DX.orbitEye(state.dist, state.cpitch, state.cyaw, target);
    const teachView = DX.lookAtLH(eye, target, [0, 1, 0]);
    const W = DX.composeWorld(1, 1, 1, 0, DX.deg(state.yaw), 0, 0, 0.5, 0);

    const endL = pane(ctx, 0, 0, mid, h, "世界空间：相机在场景里走动");
    const camL = orbitDiagram(mid, h, 11.5, 22, 48, [0, 0.5, 0]);
    const gL = begin3D(ctx, w, h, camL.view, camL.proj, 0, 0, mid, h);
    gL.grid(4, 1);
    gL.axes([0, 0, 0], 1.6, null);
    gL.cube(W, C.world, 1);
    cameraGizmo(gL, eye, target);
    gL.flush();
    endL();

    const endR = pane(ctx, mid, 0, w - mid, h, "视图空间：世界被搬到相机面前（+Z 向前）");
    const camR = diagramView(w - mid, h, [3.2, 2.4, -5.2], [0, 0, 2.2]);
    const gR = begin3D(ctx, w, h, camR.view, camR.proj, mid, 0, w - mid, h);
    gR.grid(3, 1);
    gR.axes([0, 0, 0], 1.4, null);
    const viewOfWorld = DX.mul(W, teachView);
    gR.cube(viewOfWorld, C.view, 1);
    gR.point([0, 0, 0], C.view, 4);
    gR.label([0, 0, 0], "相机原点", C.view, 16);
    gR.flush();
    endR();
  }

  function drawFrustum(ctx, w, h, state) {
    clear(ctx, w, h);
    const mid = w * 0.5;
    const aspect = 1.2;
    const fov = DX.deg(state.fov);
    const nearZ = Math.max(0.2, state.near);
    const farZ = Math.max(nearZ + 0.5, state.far);
    const isOrtho = !!state.ortho;
    const orthoH = 4;
    const P = isOrtho
      ? DX.orthoLH(orthoH * aspect, orthoH, nearZ, farZ)
      : DX.perspectiveFovLH(fov, aspect, nearZ, farZ);
    const cornersV = DX.frustumCornersView(fov, aspect, nearZ, farZ, isOrtho, orthoH);
    const cubeYaw = state.cubeYaw === undefined ? state.yaw : state.cubeYaw;
    const cubeW = DX.composeWorld(1, 1, 1, 0, DX.deg(cubeYaw), 0, 0, 0.2, (nearZ + farZ) * 0.45);

    const endL = pane(ctx, 0, 0, mid, h, isOrtho ? "正交：视锥是盒子，大小不随远近变" : "透视：视锥是金字塔，远处更宽");
    const camL = diagramView(mid, h, [5.5, 3.2, -1.2], [0, 0, (nearZ + farZ) * 0.4]);
    const gL = begin3D(ctx, w, h, camL.view, camL.proj, 0, 0, mid, h);
    gL.axes([0, 0, 0], 1.1, null);
    gL.point([0, 0, 0], C.view, 4);
    gL.label([0, 0, 0], "相机", C.view, -12);
    const idx = [[0, 1], [1, 2], [2, 3], [3, 0], [4, 5], [5, 6], [6, 7], [7, 4], [0, 4], [1, 5], [2, 6], [3, 7]];
    for (let i = 0; i < idx.length; i++) {
      gL.line(cornersV[idx[i][0]], cornersV[idx[i][1]], C.proj, 1.5);
    }
    gL.cube(cubeW, C.world, 1);
    gL.flush();
    endL();

    const endR = pane(ctx, mid, 0, w - mid, h, "乘上投影矩阵并做透视除法 → NDC 盒子");
    const camR = diagramView(w - mid, h, [2.8, 2.1, 3.4], [0, 0, 0.45]);
    const gR = begin3D(ctx, w, h, camR.view, camR.proj, mid, 0, w - mid, h);
    const ndcCube = [
      [-1, -1, 0], [1, -1, 0], [1, 1, 0], [-1, 1, 0],
      [-1, -1, 1], [1, -1, 1], [1, 1, 1], [-1, 1, 1]
    ];
    for (let i = 0; i < idx.length; i++) {
      gR.line(ndcCube[idx[i][0]], ndcCube[idx[i][1]], C.proj, 1.5);
    }
    const squeeze = state.squeeze === undefined ? 1 : state.squeeze;
    const cornersN = cornersV.map(function (p) {
      const ndc = DX.transformCoord(p, P);
      return [
        p[0] + (ndc[0] - p[0]) * squeeze,
        p[1] + (ndc[1] - p[1]) * squeeze,
        p[2] + (ndc[2] - p[2]) * squeeze
      ];
    });
    for (let i = 0; i < idx.length; i++) {
      gR.line(cornersN[idx[i][0]], cornersN[idx[i][1]], "rgba(255,183,3,0.35)", 1);
    }
    const cubePts = cubeCorners(1).map(function (p) {
      const world = DX.transformCoord(p, cubeW);
      const ndc = DX.transformCoord(world, P);
      return [
        world[0] + (ndc[0] - world[0]) * squeeze,
        world[1] + (ndc[1] - world[1]) * squeeze,
        world[2] + (ndc[2] - world[2]) * squeeze
      ];
    });
    for (let i = 0; i < CUBE_EDGES.length; i++) {
      const e = CUBE_EDGES[i];
      gR.line(cubePts[e[0]], cubePts[e[1]], C.mvp, 1.8);
    }
    gR.label([0, 1.08, 0.5], "NDC", C.proj, 0);
    gR.flush();
    endR();
  }

  function drawWvp(ctx, w, h, state) {
    clear(ctx, w, h);
    const target = [0, 0.4, 0];
    const eye = DX.orbitEye(state.dist, state.cpitch, state.cyaw, target);
    const V = DX.lookAtLH(eye, target, [0, 1, 0]);
    const aspect = w / Math.max(1, h);
    const P = state.ortho
      ? DX.orthoLH(state.dist * aspect, state.dist, 0.4, 30)
      : DX.perspectiveFovLH(DX.deg(state.fov), aspect, 0.4, 30);
    const W = DX.composeWorld(
      state.sx, state.sy, state.sz,
      DX.deg(state.pitch), DX.deg(state.yaw), DX.deg(state.roll),
      state.tx, state.ty, state.tz
    );
    const g = begin3D(ctx, w, h, V, P);
    g.grid(5, 1);
    g.axes([0, 0, 0], 1.8, null);
    g.cube(W, C.mvp, 1);
    g.axes([0, 0, 0], 0.7, W);
    const p = [state.px, state.py, state.pz];
    const pw = DX.transformCoord(p, W).slice(0, 3);
    g.point(pw, C.mvp, 6);
    g.label(pw, "追踪点", C.mvp, -12);
    g.flush();
    ctx.fillStyle = C.text;
    ctx.font = "12px Segoe UI, Microsoft YaHei, sans-serif";
    ctx.textAlign = "left";
    ctx.fillText("这就是实验台视口：顶点 × W × V × P 之后画到屏幕上。", 14, h - 14);
  }

  root.VIZ = {
    colors: C,
    fitCanvas: fitCanvas,
    drawPipeline: drawPipeline,
    drawTrack: drawTrack,
    drawTrs: drawTrs,
    drawParent: drawParent,
    drawCamera: drawCamera,
    drawFrustum: drawFrustum,
    drawWvp: drawWvp,
    planetMatrix: planetMatrix,
    satLocalMatrix: satLocalMatrix
  };
})(typeof window !== "undefined" ? window : globalThis);
