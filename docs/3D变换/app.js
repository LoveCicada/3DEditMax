(function () {
  "use strict";

  const state = {
    playing: {
      pipeline: true,
      track: true,
      trs: true,
      parent: true,
      camera: true,
      frustum: true,
      wvp: false
    },
    time: 0,
    tx: 0.85,
    ty: 0.45,
    tz: 0.25,
    yaw: 28,
    pitch: 10,
    roll: 0,
    sx: 1,
    sy: 1,
    sz: 1,
    px: 0.5,
    py: 0.5,
    pz: 0.5,
    dist: 6,
    cpitch: 22,
    cyaw: 40,
    fov: 60,
    near: 1.2,
    far: 12,
    ortho: false,
    orbit: 35,
    spin: 12,
    squeeze: 1,
    playT: 0,
    trackCamYaw: 38,
    pipelineT: 0,
    trackPose: null,
    worldOrder: "srt",
    showDir: false,
    colMajor: false
  };

  const DEFAULTS = {
    tx: 0.85, ty: 0.45, tz: 0.25, yaw: 28, pitch: 10, roll: 0,
    sx: 1, sy: 1, sz: 1, px: 0.5, py: 0.5, pz: 0.5,
    dist: 6, cpitch: 22, cyaw: 40, fov: 60, near: 1.2, far: 12,
    ortho: false, orbit: 35, spin: 12, showDir: false, worldOrder: "srt"
  };

  const RESET_KEYS = {
    track: ["px", "py", "pz"],
    trs: ["tx", "ty", "tz", "yaw", "pitch", "sx", "sy", "sz", "showDir", "worldOrder"],
    parent: ["orbit", "spin"],
    camera: ["cyaw", "cpitch", "dist"],
    frustum: ["fov", "near", "far", "ortho"],
    wvp: ["yaw", "cyaw", "fov", "tx"]
  };

  const canvases = {};

  function wrapSigned(deg) {
    let d = deg % 360;
    if (d > 180) {
      d -= 360;
    }
    if (d < -180) {
      d += 360;
    }
    return d;
  }

  function livePose() {
    return {
      sx: state.sx,
      sy: state.sy,
      sz: state.sz,
      pitch: state.pitch,
      yaw: state.yaw,
      roll: state.roll,
      tx: state.tx,
      ty: state.ty,
      tz: state.tz,
      dist: state.dist,
      cpitch: state.cpitch,
      cyaw: state.cyaw
    };
  }

  function trackPose() {
    return state.playing.track || !state.trackPose ? livePose() : state.trackPose;
  }

  function worldMatrixFrom(pose) {
    return DX.composeWorld(
      pose.sx, pose.sy, pose.sz,
      DX.deg(pose.pitch), DX.deg(pose.yaw), DX.deg(pose.roll),
      pose.tx, pose.ty, pose.tz,
      state.worldOrder
    );
  }

  function worldMatrix() {
    return worldMatrixFrom(state);
  }

  function viewMatrix() {
    const target = [0, 0.4, 0];
    const eye = DX.orbitEye(state.dist, state.cpitch, state.cyaw, target);
    return {
      eye: eye,
      target: target,
      V: DX.lookAtLH(eye, target, [0, 1, 0]),
      camW: DX.cameraWorldLH(eye, target, [0, 1, 0])
    };
  }

  function projMatrix(aspect) {
    const a = aspect || 16 / 9;
    const nearZ = Math.max(0.15, state.near);
    const farZ = Math.max(nearZ + 0.5, state.far);
    if (state.ortho) {
      const h = Math.max(2, state.dist);
      return DX.orthoLH(h * a, h, nearZ, farZ);
    }
    return DX.perspectiveFovLH(DX.deg(state.fov), a, nearZ, farZ);
  }

  function fillMatrix(table, m, kind) {
    if (!table) {
      return;
    }
    let tb = table.tBodies[0];
    if (!tb) {
      tb = document.createElement("tbody");
      table.appendChild(tb);
    }
    if (tb.rows.length !== 4) {
      tb.innerHTML = "";
      for (let r = 0; r < 4; r++) {
        const tr = document.createElement("tr");
        for (let c = 0; c < 4; c++) {
          tr.appendChild(document.createElement("td"));
        }
        tb.appendChild(tr);
      }
    }
    const rows = DX.matrixRows(m, 2, state.colMajor);
    const prev = table.getAttribute("data-sig") || "";
    const sig = rows[3].join(",");
    for (let r = 0; r < 4; r++) {
      for (let c = 0; c < 4; c++) {
        const td = tb.rows[r].cells[c];
        td.textContent = rows[r][c];
        td.className = "";
        if (kind === "world") {
          if ((!state.colMajor && r === 3 && c < 3) || (state.colMajor && c === 3 && r < 3)) {
            td.className = "hl-trs";
            if (prev && prev !== sig) {
              td.classList.add("pulse");
            }
          } else if (r < 3 && c < 3) {
            td.className = "hl-rot";
          }
        }
        if (kind === "proj" && !state.ortho && (c === 3 || r === 3)) {
          td.className = "hl-persp";
        }
      }
    }
    table.setAttribute("data-sig", sig);
  }

  function fmtVec(v, n) {
    n = n === undefined ? 4 : n;
    return DX.format4(v.slice(0, n), 2).map(function (s, i) {
      return "xyzw"[i] + " " + s;
    }).join("\n");
  }

  function fillVecs(root, tracked) {
    if (!root) {
      return;
    }
    const map = {
      model: tracked.model,
      world: tracked.world,
      view: tracked.view,
      clip: tracked.clip,
      ndc: tracked.ndc
    };
    const nodes = root.querySelectorAll("[data-vec]");
    for (let i = 0; i < nodes.length; i++) {
      const key = nodes[i].getAttribute("data-vec");
      nodes[i].textContent = fmtVec(map[key], key === "ndc" || key === "clip" ? 4 : 3);
    }
  }

  function currentTrack(aspect) {
    const W = worldMatrix();
    const cam = viewMatrix();
    const P = projMatrix(aspect);
    return {
      W: W,
      V: cam.V,
      P: P,
      cam: cam,
      tracked: DX.trackPoint([state.px, state.py, state.pz], W, cam.V, P)
    };
  }

  function bindSliders() {
    const inputs = document.querySelectorAll("[data-bind]");
    for (let i = 0; i < inputs.length; i++) {
      const el = inputs[i];
      const key = el.getAttribute("data-bind");
      const vizId = el.getAttribute("data-viz");
      if (el.type === "checkbox") {
        el.checked = !!state[key];
        el.addEventListener("change", function () {
          state[key] = el.checked;
          if (vizId) {
            state.playing[vizId] = false;
          }
          syncOutputs();
          updatePlayButtons();
        });
      } else {
        el.value = state[key];
        el.addEventListener("input", function () {
          const v = parseFloat(el.value);
          state[key] = v;
          if (key === "sy") {
            state.sz = v;
          }
          if (vizId) {
            state.playing[vizId] = false;
          }
          syncOutputs();
          updatePlayButtons();
        });
      }
    }
  }

  function syncOutputs() {
    const inputs = document.querySelectorAll("[data-bind]");
    for (let i = 0; i < inputs.length; i++) {
      const el = inputs[i];
      const key = el.getAttribute("data-bind");
      if (el.type === "checkbox") {
        el.checked = !!state[key];
        continue;
      }
      if (document.activeElement !== el) {
        el.value = state[key];
      }
      const out = el.parentElement.querySelector("output");
      if (out) {
        const n = state[key];
        out.textContent = Math.abs(n) >= 10 || n % 1 === 0 ? String(Math.round(n)) : n.toFixed(2);
      }
    }
  }

  function updatePlayButtons() {
    const buttons = document.querySelectorAll("[data-play]");
    for (let i = 0; i < buttons.length; i++) {
      const id = buttons[i].getAttribute("data-play");
      const on = !!state.playing[id];
      buttons[i].textContent = on ? "暂停" : "播放";
      buttons[i].classList.toggle("playing", on);
    }
  }

  function updateOrderButtons() {
    const buttons = document.querySelectorAll("[data-order]");
    for (let i = 0; i < buttons.length; i++) {
      buttons[i].classList.toggle("active", buttons[i].getAttribute("data-order") === state.worldOrder);
    }
  }

  function bindOrder() {
    const buttons = document.querySelectorAll("[data-order]");
    for (let i = 0; i < buttons.length; i++) {
      buttons[i].addEventListener("click", function () {
        state.worldOrder = this.getAttribute("data-order");
        state.playing.trs = false;
        updateOrderButtons();
        updatePlayButtons();
      });
    }
  }

  function bindReset() {
    const buttons = document.querySelectorAll("[data-reset]");
    for (let i = 0; i < buttons.length; i++) {
      buttons[i].addEventListener("click", function () {
        const id = this.getAttribute("data-reset");
        const keys = RESET_KEYS[id] || [];
        for (let k = 0; k < keys.length; k++) {
          const key = keys[k];
          state[key] = DEFAULTS[key];
        }
        if (keys.indexOf("sy") >= 0) {
          state.sz = state.sy;
        }
        if (state.playing[id] !== undefined) {
          state.playing[id] = false;
        }
        updateOrderButtons();
        updatePlayButtons();
        syncOutputs();
      });
    }
  }

  function matrixForCopy(which) {
    const snap = currentTrack(16 / 9);
    if (which === "world" || which === "wvpW") {
      return { m: snap.W, name: "world" };
    }
    if (which === "wvp") {
      return { m: DX.mulMany(snap.W, snap.V, snap.P), name: "worldViewProj" };
    }
    return { m: snap.W, name: "M" };
  }

  function bindCopy() {
    const buttons = document.querySelectorAll("[data-copy]");
    for (let i = 0; i < buttons.length; i++) {
      buttons[i].addEventListener("click", function () {
        const pack = matrixForCopy(this.getAttribute("data-copy"));
        const text = DX.toHlsl(pack.m, pack.name);
        const btn = this;
        const done = function () {
          const old = btn.textContent;
          btn.textContent = "已复制";
          setTimeout(function () { btn.textContent = old; }, 1200);
        };
        if (navigator.clipboard && navigator.clipboard.writeText) {
          navigator.clipboard.writeText(text).then(done).catch(function () {
            window.prompt("复制：", text);
          });
        } else {
          window.prompt("复制：", text);
        }
      });
    }
  }

  function bindMajor() {
    const btn = document.getElementById("toggle-major");
    if (!btn) {
      return;
    }
    btn.addEventListener("click", function () {
      state.colMajor = !state.colMajor;
      updateHud();
    });
  }

  function bindPlay() {
    const buttons = document.querySelectorAll("[data-play]");
    for (let i = 0; i < buttons.length; i++) {
      buttons[i].addEventListener("click", function () {
        const id = this.getAttribute("data-play");
        state.playing[id] = !state.playing[id];
        updatePlayButtons();
      });
    }
  }

  function collectCanvases() {
    const figs = document.querySelectorAll(".viz[data-viz]");
    for (let i = 0; i < figs.length; i++) {
      const id = figs[i].getAttribute("data-viz");
      const list = canvases[id] || [];
      const cvs = figs[i].querySelectorAll("canvas");
      for (let j = 0; j < cvs.length; j++) {
        list.push(cvs[j]);
      }
      canvases[id] = list;
    }
  }

  function drawId(id) {
    const list = canvases[id];
    if (!list) {
      return;
    }
    for (let i = 0; i < list.length; i++) {
      const canvas = list[i];
      const fit = VIZ.fitCanvas(canvas);
      const pose = id === "track" ? trackPose() : livePose();
      const pack = {
        ctx: fit.ctx,
        w: fit.w,
        h: fit.h,
        sx: pose.sx,
        sy: pose.sy,
        sz: pose.sz,
        pitch: pose.pitch,
        yaw: pose.yaw,
        roll: pose.roll,
        tx: pose.tx,
        ty: pose.ty,
        tz: pose.tz,
        px: state.px,
        py: state.py,
        pz: state.pz,
        dist: pose.dist,
        cpitch: pose.cpitch,
        cyaw: pose.cyaw,
        fov: state.fov,
        near: state.near,
        far: state.far,
        ortho: state.ortho,
        orbit: state.orbit,
        spin: state.spin,
        squeeze: state.squeeze,
        playT: state.playT,
        trackCamYaw: state.trackCamYaw,
        cubeYaw: state.playing.frustum ? (state.time * 24) % 360 : state.yaw,
        worldOrder: state.worldOrder,
        showDir: state.showDir
      };
      if (id === "pipeline") {
        VIZ.drawPipeline(fit.ctx, fit.w, fit.h, state.pipelineT);
      } else if (id === "track") {
        VIZ.drawTrack(fit.ctx, fit.w, fit.h, pack);
      } else if (id === "trs") {
        VIZ.drawTrs(fit.ctx, fit.w, fit.h, pack);
      } else if (id === "parent") {
        VIZ.drawParent(fit.ctx, fit.w, fit.h, pack);
      } else if (id === "camera") {
        VIZ.drawCamera(fit.ctx, fit.w, fit.h, pack);
      } else if (id === "frustum") {
        VIZ.drawFrustum(fit.ctx, fit.w, fit.h, pack);
      } else if (id === "wvp") {
        VIZ.drawWvp(fit.ctx, fit.w, fit.h, pack);
      }
    }
  }

  function updateHud() {
    const snap = currentTrack(16 / 9);
    const pose = trackPose();
    const trackSnap = DX.trackPoint(
      [state.px, state.py, state.pz],
      worldMatrixFrom(pose),
      DX.lookAtLH(
        DX.orbitEye(pose.dist, pose.cpitch, pose.cyaw, [0, 0.4, 0]),
        [0, 0.4, 0],
        [0, 1, 0]
      ),
      snap.P
    );
    fillVecs(document.getElementById("track-cards"), trackSnap);
    fillVecs(document.getElementById("wvp-cards"), snap.tracked);
    fillMatrix(document.getElementById("mat-world"), snap.W, "world");
    fillMatrix(document.getElementById("mat-view"), snap.V, "view");
    fillMatrix(document.getElementById("mat-camworld"), snap.cam.camW, "cam");
    fillMatrix(document.getElementById("mat-proj"), snap.P, "proj");
    fillMatrix(document.getElementById("mat-wvp-w"), snap.W, "world");
    fillMatrix(document.getElementById("mat-wvp-v"), snap.V, "view");
    fillMatrix(document.getElementById("mat-wvp-p"), snap.P, "proj");
    fillMatrix(document.getElementById("mat-wvp"), DX.mulMany(snap.W, snap.V, snap.P), "mvp");
    const clip = snap.tracked.clip;
    const ndc = snap.tracked.ndc;
    const wEl = document.getElementById("proj-w");
    const cEl = document.getElementById("proj-clip");
    const nEl = document.getElementById("proj-ndc");
    if (cEl) {
      cEl.textContent = fmtVec(clip, 4);
    }
    if (wEl) {
      const w = clip[3];
      wEl.textContent = "w " + DX.format4([w], 2)[0] + "\nxyzw / w → NDC";
    }
    if (nEl) {
      nEl.textContent = fmtVec(ndc, 4);
    }
    const majorBtn = document.getElementById("toggle-major");
    if (majorBtn) {
      majorBtn.textContent = state.colMajor ? "行主序显示" : "列主序显示";
    }
  }

  function tick(now) {
    if (!tick.start) {
      tick.start = now;
      tick.prev = now;
    }
    const dt = Math.min(0.05, (now - tick.prev) / 1000);
    tick.prev = now;
    state.time = (now - tick.start) / 1000;
    state.playT = state.time;

    if (state.playing.pipeline) {
      state.pipelineT = (state.time % 8) / 8;
    }
    if (state.playing.track) {
      state.trackCamYaw = wrapSigned(state.trackCamYaw + dt * 18);
      state.trackPose = livePose();
    }
    if (state.playing.trs) {
      state.yaw = wrapSigned(state.yaw + dt * 22);
    }
    if (state.playing.parent) {
      state.orbit = (state.orbit + dt * 40) % 360;
      state.spin = (state.spin + dt * 28) % 360;
    }
    if (state.playing.camera) {
      state.cyaw = wrapSigned(state.cyaw + dt * 18);
    }
    if (state.playing.frustum) {
      state.squeeze = 0.5 + 0.5 * Math.sin(state.time * 1.1);
    } else {
      state.squeeze = 1;
    }
    if (state.playing.wvp && !state.playing.camera) {
      state.cyaw = wrapSigned(state.cyaw + dt * 14);
    }

    syncOutputs();
    drawId("pipeline");
    drawId("track");
    drawId("trs");
    drawId("parent");
    drawId("camera");
    drawId("frustum");
    drawId("wvp");
    updateHud();
    requestAnimationFrame(tick);
  }

  function bindToc() {
    const links = document.querySelectorAll(".toc a");
    const sections = [];
    for (let i = 0; i < links.length; i++) {
      const id = links[i].getAttribute("href").slice(1);
      const el = document.getElementById(id);
      if (el) {
        sections.push({ el: el, link: links[i] });
      }
    }
    function onScroll() {
      let current = sections[0];
      for (let i = 0; i < sections.length; i++) {
        if (sections[i].el.getBoundingClientRect().top < 120) {
          current = sections[i];
        }
      }
      for (let i = 0; i < sections.length; i++) {
        sections[i].link.classList.toggle("active", sections[i] === current);
      }
    }
    document.addEventListener("scroll", onScroll, { passive: true });
    onScroll();
  }

  collectCanvases();
  bindSliders();
  bindPlay();
  bindOrder();
  bindReset();
  bindCopy();
  bindMajor();
  bindToc();
  syncOutputs();
  updatePlayButtons();
  updateOrderButtons();
  window.addEventListener("resize", function () {
    drawId("pipeline");
    drawId("track");
    drawId("trs");
    drawId("parent");
    drawId("camera");
    drawId("frustum");
    drawId("wvp");
  });
  requestAnimationFrame(tick);
})();
