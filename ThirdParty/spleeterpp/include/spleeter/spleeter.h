#ifndef SPEETER_SPLEETER_H_
#define SPEETER_SPLEETER_H_

#include <string>
#include <memory>
#include <unordered_set>
#include <Eigen/Core>

#include "spleeter_common/spleeter_common.h"

namespace spleeter {

/// Split the input waveform into vocals and accompaniment
/// \param input the input waveform
/// \param vocals the extracted vocals waveform
/// \param accompaniment the extracted accompaniment waveform
/// \param err an error code that gets set if something goes wrong.
/// std::errc::protocol_error is obtained when the model hasn't been properly
/// initialized
void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment,
           std::error_code &err);

/// Split the input waveform into vocals, drums, bass and other
/// \param input the input waveform
/// \param vocals the extracted vocals waveform
/// \param drums the extracted drums waveform
/// \param bass the extracted bass waveform
/// \param other the extracted other waveform
/// \param err an error code that gets set if something goes wrong.
/// std::errc::protocol_error is obtained when the model hasn't been properly
/// initialized
void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other, std::error_code &err);


/// Split the input waveform into vocals, drums, bass, piano and other
/// \param input the input waveform
/// \param vocals the extracted vocals waveform
/// \param drums the extracted drums waveform
/// \param bass the extracted bass waveform
/// \param piano the extracted piano waveform
/// \param other the extracted other waveform
/// \param err an error code that gets set if something goes wrong.
/// std::errc::protocol_error is obtained when the model hasn't been properly
/// initialized
void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other,
           std::error_code &err);

} // namespace spleeter

#endif // SPEETER_SPLEETER_H_
