#include <pangolin/pangolin.h>

#include <pangolin/utils/condition_variable.h>
#include <pangolin/utils/shared_memory_buffer.h>
#include <pangolin/utils/timer.h>

#include <cmath>
#include <cstdint>

// This sample acts as a soft camera. It writes a pattern of GRAY8 pixels to the
// shared memory space. It can be seen in Pangolin's SimpleVideo sample using
// the shmem://example?size=640x480 video URI.

using namespace pangolin;

uint8_t generate_value(basetime t, basetime start)
{
  int64_t us = TimeDiff_us(start, t);

  // 10s sinusoid
  double d = 10./(M_PI) * ((double)us/1000000);
  d = std::sin(d);
  d = d*128 + 128;
  return static_cast<uint8_t>(d);
}

int main(int argc, char *argv[])
{
  std::string shmem_name = "/example";

  auto shmem_buffer = create_named_shared_memory_buffer(shmem_name, 640 * 480);
  if (!shmem_buffer) {
    perror("Unable to create shared memory buffer");
    exit(1);
  }

  auto cond_name = shmem_name + "_cond";
  auto buffer_full = create_named_condition_variable(cond_name);

  basetime start = TimeNow();
  basetime d = {0};
  d.tv_usec = 33333;

  // Sit in a loop and write gray values based on some timing pattern.
  basetime t = start;
  while (true) {
    basetime nt = TimeAdd(t, d);

    shmem_buffer->lock();
    uint8_t *ptr = shmem_buffer->ptr();
    uint8_t value = generate_value(t, start);

    for (int i = 0; i < 640*480; ++i) {
      ptr[i] = value;
    }

    shmem_buffer->unlock();
    buffer_full->signal();

    t = WaitUntil(nt);
  }
}
