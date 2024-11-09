#include "core1_main.h"

#include "lib/sample_data/TransactionBuffer.h"
#include "modules/ad7887/ad7887.h"
#include "modules/rf_power_meter/converter.h"
#include <cinttypes>
#include <cstdio>
#include <hardware/gpio.h>
#include <pico/multicore.h>

struct Sampling
{
  Ad7887Sample     last_sample{};
  Average255Uint16 average{};
};

using namespace rfpm;
static TransactionBuffer *out_buffer{ nullptr };
static Sampling           sampling{};

static void init()
{

  const uint32_t received_value{ multicore_fifo_pop_blocking() };
  printf("C1I init synchronized on signal %" PRIu32 "\n", received_value);

  printf("C1I init_core%" PRIu8 " ...\n", get_core_num());
  ad7887_init(sampling.last_sample);
  printf("C1I init_core%" PRIu8 " done\n", get_core_num());
}

void core1_init(TransactionBuffer &out_buff) { out_buffer = &out_buff; }

static ConvertedSample convert_sample(const AveragedUint16 &sample)
{
  constexpr FrequencyBand    band{ FrequencyBand::GHz_0_9 };
  constexpr CorrectionValues correction{ .k0 = 0, .k1 = 1 };

  const UnderlyingConversionType dbv_value{ dbvCorrectedFromDbv(dbvFromVAdc(voltFrom12bitAdc(sample.value, AD7887_V_REF), band), correction) };
  const UnderlyingConversionType dbv_avg{ dbvCorrectedFromDbv(dbvFromVAdc(voltFrom12bitAdc(sample.avg, AD7887_V_REF), band), correction) };
  const UnderlyingConversionType dbv_min{ dbvCorrectedFromDbv(dbvFromVAdc(voltFrom12bitAdc(sample.min, AD7887_V_REF), band), correction) };
  const UnderlyingConversionType dbv_max{ dbvCorrectedFromDbv(dbvFromVAdc(voltFrom12bitAdc(sample.max, AD7887_V_REF), band), correction) };

  return {
    .value_dbv{
               .value{ .value = dbv_value, .lin = si::Linearity::Db10, .scale = si::Scale::TimesOne, .unit = si::Unit::Volt },
               .avg{ .value = dbv_avg, .lin = si::Linearity::Db10, .scale = si::Scale::TimesOne, .unit = si::Unit::Volt },
               .min{ .value = dbv_min, .lin = si::Linearity::Db10, .scale = si::Scale::TimesOne, .unit = si::Unit::Volt },
               .max{ .value = dbv_max, .lin = si::Linearity::Db10, .scale = si::Scale::TimesOne, .unit = si::Unit::Volt },
               },
    .value_linearv{ .value{ toLinearV(dbv_value) }, .avg{ toLinearV(dbv_avg) }, .min{ toLinearV(dbv_min) }, .max{ toLinearV(dbv_max) } }
  };
}

[[noreturn]] void core1_main()
{
  assert(1 == get_core_num());

  init();
  alarm_pool_create_with_unused_hardware_alarm(16);

  ad7887_start();
  while (true)
  {
    if (sampling.last_sample.is_data_ready)
    {
      sampling.last_sample.is_data_ready = false;
      sampling.average.put(sampling.last_sample.data.asSampleRegister16b.raw12b);
      TransactionData td{ .timestamp_ms = timer_hw->timerawl,
                          .probe_temperature{},
                          .converted_sample{ convert_sample(sampling.average.get()) } };
      out_buffer->write(td);
    }
  }
}
