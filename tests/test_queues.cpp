#include "test_harness.h"
#include "render/CommandQueue.h"
#include "render/FeedbackQueue.h"
#include "render/RenderThread.h"
#include "core/SnapshotBuffer.h"
#include <chrono>
#include <thread>

void runQueueTests() {
  CommandQueue cq;
  RenderCommand c;
  TEST_CHECK(cq.tryPop(&c) == false);
  RenderCommand in;
  in.type = CmdStop;
  in.hwnd = 0;
  in.width = 0;
  in.height = 0;
  cq.push(in);
  TEST_CHECK(cq.tryPop(&c) == true);
  TEST_CHECK(c.type == CmdStop);

  FeedbackQueue fq;
  FeedbackItem f;
  TEST_CHECK(fq.tryPop(&f) == false);
  FeedbackItem fin;
  fin.kind = FbError;
  fin.text = "boom";
  fin.ms = 1.5f;
  fq.push(fin);
  TEST_CHECK(fq.tryPop(&f) == true);
  TEST_CHECK(f.kind == FbError);
  TEST_CHECK(f.text == "boom");

  SnapshotBuffer buf;
  StateSnapshot a = stateSnapshotDefault();
  a.teaching.camDistance = 9.f;
  a.viewportW = 640;
  buf.publish(a);
  StateSnapshot b = buf.consume();
  TEST_CHECK(b.teaching.camDistance == 9.f);
  TEST_CHECK(b.viewportW == 640);
}

void runRenderThreadTests() {
  RenderThread rt;
  rt.start();
  rt.requestStopAndJoin();
  TEST_CHECK(true);
}
