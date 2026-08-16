#include "teach/JsonIo.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

void appendNum(std::string* o, float v) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.9g", static_cast<double>(v));
  *o += buf;
}

void appendInt(std::string* o, int v) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%d", v);
  *o += buf;
}

void appendVec3(std::string* o, const float v[3]) {
  *o += '[';
  appendNum(o, v[0]);
  *o += ',';
  appendNum(o, v[1]);
  *o += ',';
  appendNum(o, v[2]);
  *o += ']';
}

void appendObject(std::string* o, const TeachingObject& obj) {
  *o += "{\"mesh\":";
  appendInt(o, static_cast<int>(obj.mesh));
  *o += ",\"pos\":";
  appendVec3(o, obj.trs.pos);
  *o += ",\"pitchDeg\":";
  appendNum(o, obj.trs.pitchDeg);
  *o += ",\"yawDeg\":";
  appendNum(o, obj.trs.yawDeg);
  *o += ",\"rollDeg\":";
  appendNum(o, obj.trs.rollDeg);
  *o += ",\"scale\":";
  appendVec3(o, obj.trs.scale);
  *o += '}';
}

struct Parser {
  const char* p;
  const char* end;
  bool ok;

  explicit Parser(const std::string& in)
      : p(in.c_str())
      , end(in.c_str() + in.size())
      , ok(true) {
  }

  void skip() {
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
      ++p;
    }
  }

  bool eat(char c) {
    skip();
    if (p < end && *p == c) {
      ++p;
      return true;
    }
    ok = false;
    return false;
  }

  bool peek(char c) {
    skip();
    return p < end && *p == c;
  }

  bool parseString(std::string* out) {
    skip();
    if (p >= end || *p != '"') {
      ok = false;
      return false;
    }
    ++p;
    const char* start = p;
    while (p < end && *p != '"') {
      ++p;
    }
    if (p >= end) {
      ok = false;
      return false;
    }
    out->assign(start, p);
    ++p;
    return true;
  }

  bool parseNumber(double* v) {
    skip();
    char* next = 0;
    *v = std::strtod(p, &next);
    if (next == p) {
      ok = false;
      return false;
    }
    p = next;
    return true;
  }

  bool parseInt(int* v) {
    double d = 0.0;
    if (!parseNumber(&d)) {
      return false;
    }
    *v = static_cast<int>(d);
    return true;
  }

  bool parseFloat(float* v) {
    double d = 0.0;
    if (!parseNumber(&d)) {
      return false;
    }
    *v = static_cast<float>(d);
    return true;
  }

  bool parseBool(bool* v) {
    skip();
    if (p + 4 <= end && std::memcmp(p, "true", 4) == 0) {
      p += 4;
      *v = true;
      return true;
    }
    if (p + 5 <= end && std::memcmp(p, "false", 5) == 0) {
      p += 5;
      *v = false;
      return true;
    }
    ok = false;
    return false;
  }

  bool parseVec3(float v[3]) {
    if (!eat('[')) {
      return false;
    }
    if (!parseFloat(&v[0]) || !eat(',') || !parseFloat(&v[1]) || !eat(',') ||
        !parseFloat(&v[2]) || !eat(']')) {
      return false;
    }
    return true;
  }

  bool skipValue();

  bool parseTeachingObject(TeachingObject* obj) {
    if (!eat('{')) {
      return false;
    }
    unsigned seen = 0;
    bool first = true;
    while (ok && !peek('}')) {
      if (!first && !eat(',')) {
        return false;
      }
      first = false;
      std::string key;
      if (!parseString(&key) || !eat(':')) {
        return false;
      }
      if (key == "mesh") {
        int mesh = 0;
        if (!parseInt(&mesh)) {
          return false;
        }
        obj->mesh = static_cast<MeshId>(mesh);
        seen |= 1u;
      } else if (key == "pos") {
        if (!parseVec3(obj->trs.pos)) {
          return false;
        }
        seen |= 2u;
      } else if (key == "pitchDeg") {
        if (!parseFloat(&obj->trs.pitchDeg)) {
          return false;
        }
        seen |= 4u;
      } else if (key == "yawDeg") {
        if (!parseFloat(&obj->trs.yawDeg)) {
          return false;
        }
        seen |= 8u;
      } else if (key == "rollDeg") {
        if (!parseFloat(&obj->trs.rollDeg)) {
          return false;
        }
        seen |= 16u;
      } else if (key == "scale") {
        if (!parseVec3(obj->trs.scale)) {
          return false;
        }
        seen |= 32u;
      } else if (!skipValue()) {
        return false;
      }
    }
    if (!eat('}')) {
      return false;
    }
    if (seen != 63u) {
      ok = false;
      return false;
    }
    return true;
  }

  bool parseObjects(TeachingObject objects[3]) {
    if (!eat('[')) {
      return false;
    }
    int count = 0;
    bool first = true;
    while (ok && !peek(']')) {
      if (!first && !eat(',')) {
        return false;
      }
      first = false;
      if (count >= 3) {
        ok = false;
        return false;
      }
      if (!parseTeachingObject(&objects[count])) {
        return false;
      }
      ++count;
    }
    if (!eat(']')) {
      return false;
    }
    if (count != 3) {
      ok = false;
      return false;
    }
    return true;
  }
};

bool Parser::skipValue() {
  skip();
  if (p >= end) {
    ok = false;
    return false;
  }
  if (*p == '{') {
    eat('{');
    bool first = true;
    while (ok && !peek('}')) {
      if (!first && !eat(',')) {
        return false;
      }
      first = false;
      std::string key;
      if (!parseString(&key) || !eat(':') || !skipValue()) {
        return false;
      }
    }
    return eat('}');
  }
  if (*p == '[') {
    eat('[');
    bool first = true;
    while (ok && !peek(']')) {
      if (!first && !eat(',')) {
        return false;
      }
      first = false;
      if (!skipValue()) {
        return false;
      }
    }
    return eat(']');
  }
  if (*p == '"') {
    std::string dummy;
    return parseString(&dummy);
  }
  if (*p == 't' || *p == 'f') {
    bool b = false;
    return parseBool(&b);
  }
  double d = 0.0;
  return parseNumber(&d);
}

enum {
  F_layout = 1 << 0,
  F_shading = 1 << 1,
  F_camDistance = 1 << 2,
  F_camPitchDeg = 1 << 3,
  F_camYawDeg = 1 << 4,
  F_proj = 1 << 5,
  F_fovDeg = 1 << 6,
  F_aspect = 1 << 7,
  F_aspectFollowViewport = 1 << 8,
  F_nearZ = 1 << 9,
  F_farZ = 1 << 10,
  F_trackModel = 1 << 11,
  F_objects = 1 << 12
};

const unsigned kRequired = F_layout | F_shading | F_camDistance | F_camPitchDeg |
                           F_camYawDeg | F_proj | F_fovDeg | F_aspect |
                           F_aspectFollowViewport | F_nearZ | F_farZ |
                           F_trackModel | F_objects;

}  // namespace

bool teachingToJson(const TeachingState& s, std::string* out) {
  if (!out) {
    return false;
  }
  std::string& o = *out;
  o.clear();
  o += "{\n";
  o += "  \"layout\":";
  appendInt(&o, static_cast<int>(s.layout));
  o += ",\n  \"shading\":";
  appendInt(&o, static_cast<int>(s.shading));
  o += ",\n  \"camDistance\":";
  appendNum(&o, s.camDistance);
  o += ",\n  \"camPitchDeg\":";
  appendNum(&o, s.camPitchDeg);
  o += ",\n  \"camYawDeg\":";
  appendNum(&o, s.camYawDeg);
  o += ",\n  \"camTarget\":";
  appendVec3(&o, s.camTarget);
  o += ",\n  \"proj\":";
  appendInt(&o, static_cast<int>(s.proj));
  o += ",\n  \"fovDeg\":";
  appendNum(&o, s.fovDeg);
  o += ",\n  \"aspect\":";
  appendNum(&o, s.aspect);
  o += ",\n  \"aspectFollowViewport\":";
  o += (s.aspectFollowViewport ? "true" : "false");
  o += ",\n  \"nearZ\":";
  appendNum(&o, s.nearZ);
  o += ",\n  \"farZ\":";
  appendNum(&o, s.farZ);
  o += ",\n  \"trackModel\":";
  appendVec3(&o, s.trackModel);
  o += ",\n  \"objects\":[\n    ";
  appendObject(&o, s.objects[0]);
  o += ",\n    ";
  appendObject(&o, s.objects[1]);
  o += ",\n    ";
  appendObject(&o, s.objects[2]);
  o += "\n  ]\n}\n";
  return true;
}

bool teachingFromJson(const std::string& in, TeachingState* out) {
  if (!out) {
    return false;
  }
  TeachingState tmp = teachingStateDefault();
  Parser pr(in);
  if (!pr.eat('{')) {
    return false;
  }
  unsigned seen = 0;
  bool first = true;
  while (pr.ok && !pr.peek('}')) {
    if (!first && !pr.eat(',')) {
      return false;
    }
    first = false;
    std::string key;
    if (!pr.parseString(&key) || !pr.eat(':')) {
      return false;
    }
    if (key == "layout") {
      int v = 0;
      if (!pr.parseInt(&v)) {
        return false;
      }
      tmp.layout = static_cast<LayoutMode>(v);
      seen |= F_layout;
    } else if (key == "shading") {
      int v = 0;
      if (!pr.parseInt(&v)) {
        return false;
      }
      tmp.shading = static_cast<ShadingMode>(v);
      seen |= F_shading;
    } else if (key == "camDistance") {
      if (!pr.parseFloat(&tmp.camDistance)) {
        return false;
      }
      seen |= F_camDistance;
    } else if (key == "camPitchDeg") {
      if (!pr.parseFloat(&tmp.camPitchDeg)) {
        return false;
      }
      seen |= F_camPitchDeg;
    } else if (key == "camYawDeg") {
      if (!pr.parseFloat(&tmp.camYawDeg)) {
        return false;
      }
      seen |= F_camYawDeg;
    } else if (key == "camTarget") {
      if (!pr.parseVec3(tmp.camTarget)) {
        return false;
      }
    } else if (key == "proj") {
      int v = 0;
      if (!pr.parseInt(&v)) {
        return false;
      }
      tmp.proj = static_cast<ProjMode>(v);
      seen |= F_proj;
    } else if (key == "fovDeg") {
      if (!pr.parseFloat(&tmp.fovDeg)) {
        return false;
      }
      seen |= F_fovDeg;
    } else if (key == "aspect") {
      if (!pr.parseFloat(&tmp.aspect)) {
        return false;
      }
      seen |= F_aspect;
    } else if (key == "aspectFollowViewport") {
      if (!pr.parseBool(&tmp.aspectFollowViewport)) {
        return false;
      }
      seen |= F_aspectFollowViewport;
    } else if (key == "nearZ") {
      if (!pr.parseFloat(&tmp.nearZ)) {
        return false;
      }
      seen |= F_nearZ;
    } else if (key == "farZ") {
      if (!pr.parseFloat(&tmp.farZ)) {
        return false;
      }
      seen |= F_farZ;
    } else if (key == "trackModel") {
      if (!pr.parseVec3(tmp.trackModel)) {
        return false;
      }
      seen |= F_trackModel;
    } else if (key == "objects") {
      if (!pr.parseObjects(tmp.objects)) {
        return false;
      }
      seen |= F_objects;
    } else if (!pr.skipValue()) {
      return false;
    }
  }
  if (!pr.eat('}')) {
    return false;
  }
  pr.skip();
  if (!pr.ok || pr.p != pr.end || seen != kRequired) {
    return false;
  }
  *out = tmp;
  return true;
}
