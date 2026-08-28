/**
 * DirectX 风格 4×4 矩阵（与 3DEditMax / DirectXMath 对齐）
 *
 * - 行向量：p' = p * M
 * - 存储：行主序，m[row * 4 + col]
 * - 平移在最后一行 (m12, m13, m14)
 * - 左手、Y-up；世界矩阵 W = S * R * T
 * - 投影深度 NDC z ∈ [0, 1]
 */
(function (root) {
  "use strict";

  function identity() {
    return new Float32Array([
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    ]);
  }

  function copy(m) {
    return new Float32Array(m);
  }

  function mul(a, b) {
    const c = new Float32Array(16);
    for (let r = 0; r < 4; r++) {
      const r0 = a[r * 4];
      const r1 = a[r * 4 + 1];
      const r2 = a[r * 4 + 2];
      const r3 = a[r * 4 + 3];
      c[r * 4]     = r0 * b[0] + r1 * b[4] + r2 * b[8]  + r3 * b[12];
      c[r * 4 + 1] = r0 * b[1] + r1 * b[5] + r2 * b[9]  + r3 * b[13];
      c[r * 4 + 2] = r0 * b[2] + r1 * b[6] + r2 * b[10] + r3 * b[14];
      c[r * 4 + 3] = r0 * b[3] + r1 * b[7] + r2 * b[11] + r3 * b[15];
    }
    return c;
  }

  function mulMany() {
    let m = arguments[0];
    for (let i = 1; i < arguments.length; i++) {
      m = mul(m, arguments[i]);
    }
    return m;
  }

  function translation(x, y, z) {
    return new Float32Array([
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      x, y, z, 1
    ]);
  }

  function scaling(x, y, z) {
    return new Float32Array([
      x, 0, 0, 0,
      0, y, 0, 0,
      0, 0, z, 0,
      0, 0, 0, 1
    ]);
  }

  function rotationX(rad) {
    const c = Math.cos(rad);
    const s = Math.sin(rad);
    return new Float32Array([
      1, 0, 0, 0,
      0, c, s, 0,
      0, -s, c, 0,
      0, 0, 0, 1
    ]);
  }

  function rotationY(rad) {
    const c = Math.cos(rad);
    const s = Math.sin(rad);
    return new Float32Array([
      c, 0, -s, 0,
      0, 1, 0, 0,
      s, 0, c, 0,
      0, 0, 0, 1
    ]);
  }

  function rotationZ(rad) {
    const c = Math.cos(rad);
    const s = Math.sin(rad);
    return new Float32Array([
      c, s, 0, 0,
      -s, c, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    ]);
  }

  /** 与 XMMatrixRotationRollPitchYaw 相同：先 Roll(Z)，再 Pitch(X)，再 Yaw(Y)。 */
  function rotationRollPitchYaw(pitch, yaw, roll) {
    return mulMany(rotationZ(roll), rotationX(pitch), rotationY(yaw));
  }

  function composeWorld(sx, sy, sz, pitch, yaw, roll, tx, ty, tz, order) {
    const S = scaling(sx, sy, sz);
    const R = rotationRollPitchYaw(pitch, yaw, roll);
    const T = translation(tx, ty, tz);
    if (order === "trs") {
      return mulMany(T, R, S);
    }
    return mulMany(S, R, T);
  }

  function deg(d) {
    return d * Math.PI / 180;
  }

  function radToDeg(r) {
    return r * 180 / Math.PI;
  }

  function vec3(x, y, z) {
    return [x, y, z];
  }

  function add3(a, b) {
    return [a[0] + b[0], a[1] + b[1], a[2] + b[2]];
  }

  function sub3(a, b) {
    return [a[0] - b[0], a[1] - b[1], a[2] - b[2]];
  }

  function scale3(a, s) {
    return [a[0] * s, a[1] * s, a[2] * s];
  }

  function dot3(a, b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  }

  function len3(a) {
    return Math.sqrt(dot3(a, a));
  }

  function normalize3(a) {
    const l = len3(a);
    if (l < 1e-12) {
      return [0, 0, 0];
    }
    return [a[0] / l, a[1] / l, a[2] / l];
  }

  function cross3(a, b) {
    return [
      a[1] * b[2] - a[2] * b[1],
      a[2] * b[0] - a[0] * b[2],
      a[0] * b[1] - a[1] * b[0]
    ];
  }

  function transform(p, m, w) {
    if (w === undefined) {
      w = 1;
    }
    const x = p[0];
    const y = p[1];
    const z = p[2];
    return [
      x * m[0] + y * m[4] + z * m[8]  + w * m[12],
      x * m[1] + y * m[5] + z * m[9]  + w * m[13],
      x * m[2] + y * m[6] + z * m[10] + w * m[14],
      x * m[3] + y * m[7] + z * m[11] + w * m[15]
    ];
  }

  function transformCoord(p, m) {
    const c = transform(p, m, 1);
    if (Math.abs(c[3]) < 1e-8) {
      return [c[0], c[1], c[2], c[3]];
    }
    return [c[0] / c[3], c[1] / c[3], c[2] / c[3], 1];
  }

  function lookAtLH(eye, at, up) {
    const zaxis = normalize3(sub3(at, eye));
    const xaxis = normalize3(cross3(up, zaxis));
    const yaxis = cross3(zaxis, xaxis);
    return new Float32Array([
      xaxis[0], yaxis[0], zaxis[0], 0,
      xaxis[1], yaxis[1], zaxis[1], 0,
      xaxis[2], yaxis[2], zaxis[2], 0,
      -dot3(xaxis, eye), -dot3(yaxis, eye), -dot3(zaxis, eye), 1
    ]);
  }

  function cameraWorldLH(eye, at, up) {
    const zaxis = normalize3(sub3(at, eye));
    const xaxis = normalize3(cross3(up, zaxis));
    const yaxis = cross3(zaxis, xaxis);
    return new Float32Array([
      xaxis[0], xaxis[1], xaxis[2], 0,
      yaxis[0], yaxis[1], yaxis[2], 0,
      zaxis[0], zaxis[1], zaxis[2], 0,
      eye[0], eye[1], eye[2], 1
    ]);
  }

  function orbitEye(distance, pitchDeg, yawDeg, target) {
    const pitch = deg(pitchDeg);
    const yaw = deg(yawDeg);
    const offset = [
      distance * Math.sin(yaw) * Math.cos(pitch),
      distance * Math.sin(pitch),
      distance * Math.cos(yaw) * Math.cos(pitch)
    ];
    return add3(target, offset);
  }

  function perspectiveFovLH(fovYRad, aspect, nearZ, farZ) {
    const height = 1 / Math.tan(fovYRad * 0.5);
    const width = height / aspect;
    const range = farZ / (farZ - nearZ);
    return new Float32Array([
      width, 0, 0, 0,
      0, height, 0, 0,
      0, 0, range, 1,
      0, 0, -range * nearZ, 0
    ]);
  }

  function orthoLH(width, height, nearZ, farZ) {
    const range = 1 / (farZ - nearZ);
    return new Float32Array([
      2 / width, 0, 0, 0,
      0, 2 / height, 0, 0,
      0, 0, range, 0,
      0, 0, -nearZ * range, 1
    ]);
  }

  function invert(src) {
    const a = Array.prototype.slice.call(src);
    const inv = [
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    ];
    for (let i = 0; i < 4; i++) {
      let pivot = i;
      let best = Math.abs(a[i * 4 + i]);
      for (let r = i + 1; r < 4; r++) {
        const v = Math.abs(a[r * 4 + i]);
        if (v > best) {
          best = v;
          pivot = r;
        }
      }
      if (best < 1e-10) {
        return identity();
      }
      if (pivot !== i) {
        for (let c = 0; c < 4; c++) {
          let tmp = a[i * 4 + c];
          a[i * 4 + c] = a[pivot * 4 + c];
          a[pivot * 4 + c] = tmp;
          tmp = inv[i * 4 + c];
          inv[i * 4 + c] = inv[pivot * 4 + c];
          inv[pivot * 4 + c] = tmp;
        }
      }
      const diag = a[i * 4 + i];
      for (let c = 0; c < 4; c++) {
        a[i * 4 + c] /= diag;
        inv[i * 4 + c] /= diag;
      }
      for (let r = 0; r < 4; r++) {
        if (r === i) {
          continue;
        }
        const f = a[r * 4 + i];
        for (let c = 0; c < 4; c++) {
          a[r * 4 + c] -= f * a[i * 4 + c];
          inv[r * 4 + c] -= f * inv[i * 4 + c];
        }
      }
    }
    return new Float32Array(inv);
  }

  function projectToScreen(pWorld, view, proj, viewportW, viewportH) {
    const clip = transform(pWorld, mul(view, proj), 1);
    if (Math.abs(clip[3]) < 1e-8) {
      return null;
    }
    const ndcX = clip[0] / clip[3];
    const ndcY = clip[1] / clip[3];
    const ndcZ = clip[2] / clip[3];
    return {
      x: (ndcX * 0.5 + 0.5) * viewportW,
      y: (1 - (ndcY * 0.5 + 0.5)) * viewportH,
      z: ndcZ,
      clip: clip,
      ndc: [ndcX, ndcY, ndcZ, 1]
    };
  }

  function trackPoint(pModel, w, v, p) {
    const model = [pModel[0], pModel[1], pModel[2], 1];
    const world = transform(pModel, w, 1);
    const view = transform(world, v, 1);
    const clip = transform(view, p, 1);
    let ndc = clip.slice();
    if (Math.abs(clip[3]) > 1e-8) {
      ndc = [clip[0] / clip[3], clip[1] / clip[3], clip[2] / clip[3], 1];
    }
    return { model: model, world: world, view: view, clip: clip, ndc: ndc };
  }

  function frustumCornersView(fovYRad, aspect, nearZ, farZ, isOrtho, orthoHeight) {
    let nw;
    let nh;
    let fw;
    let fh;
    if (isOrtho) {
      nh = orthoHeight * 0.5;
      nw = nh * aspect;
      fw = nw;
      fh = nh;
    } else {
      const t = Math.tan(fovYRad * 0.5);
      nh = nearZ * t;
      nw = nh * aspect;
      fh = farZ * t;
      fw = fh * aspect;
    }
    return [
      [-nw, -nh, nearZ], [nw, -nh, nearZ], [nw, nh, nearZ], [-nw, nh, nearZ],
      [-fw, -fh, farZ], [fw, -fh, farZ], [fw, fh, farZ], [-fw, fh, farZ]
    ];
  }

  function format4(v, digits) {
    if (digits === undefined) {
      digits = 2;
    }
    return v.map(function (n) {
      const x = Number(n);
      if (!isFinite(x)) {
        return "—";
      }
      const s = x.toFixed(digits);
      return s === "-0.00" || s === "-0.000" ? (0).toFixed(digits) : s;
    });
  }

  function matrixRows(m, digits, colMajor) {
    const rows = [];
    for (let r = 0; r < 4; r++) {
      const cells = [];
      for (let c = 0; c < 4; c++) {
        const i = colMajor ? c * 4 + r : r * 4 + c;
        cells.push(format4([m[i]], digits)[0]);
      }
      rows.push(cells);
    }
    return rows;
  }

  function toHlsl(m, name) {
    const rows = matrixRows(m, 4, false);
    const body = rows.map(function (row) {
      return "  " + row.map(function (s) { return s + "f"; }).join(", ");
    }).join(",\n");
    return "float4x4 " + (name || "M") + " = {\n" + body + "\n};";
  }

  const DX = {
    identity: identity,
    copy: copy,
    mul: mul,
    mulMany: mulMany,
    translation: translation,
    scaling: scaling,
    rotationX: rotationX,
    rotationY: rotationY,
    rotationZ: rotationZ,
    rotationRollPitchYaw: rotationRollPitchYaw,
    composeWorld: composeWorld,
    deg: deg,
    radToDeg: radToDeg,
    vec3: vec3,
    add3: add3,
    sub3: sub3,
    scale3: scale3,
    dot3: dot3,
    len3: len3,
    normalize3: normalize3,
    cross3: cross3,
    transform: transform,
    transformCoord: transformCoord,
    lookAtLH: lookAtLH,
    cameraWorldLH: cameraWorldLH,
    orbitEye: orbitEye,
    perspectiveFovLH: perspectiveFovLH,
    orthoLH: orthoLH,
    invert: invert,
    projectToScreen: projectToScreen,
    trackPoint: trackPoint,
    frustumCornersView: frustumCornersView,
    format4: format4,
    matrixRows: matrixRows,
    toHlsl: toHlsl
  };

  root.DX = DX;
  if (typeof module !== "undefined" && module.exports) {
    module.exports = DX;
  }
})(typeof window !== "undefined" ? window : globalThis);
