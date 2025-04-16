#ifndef MODEL_HELPERS_H
#define MODEL_HELPERS_H

#include <Chirale_TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

// Include the model byte array
#include "model.h"

// TensorFlow Lite variables
constexpr int tensorArenaSize = 1024 * 64; // Adjust based on your model size
uint8_t tensor_arena[tensorArenaSize];
tflite::MicroInterpreter* interpreter;
tflite::AllOpsResolver resolver;
const tflite::Model* tflModel;

TfLiteTensor* tflInputTensor;
TfLiteTensor* tflOutputTensor;

// Function to initialize TensorFlow Lite and model
void setupTFLite() {
  Serial.println("Initializing TensorFlow Lite...");

  // Load the TensorFlow Lite model
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version mismatch!");
    return;
  }

  // Create the interpreter
  interpreter = new tflite::MicroInterpreter(tflModel, resolver, tensor_arena, sizeof(tensor_arena));
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Failed to allocate tensors!");
    return;
  }

  // Get the input and output tensors
  tflInputTensor = interpreter->input(0);
  tflOutputTensor = interpreter->output(0);

  Serial.println("TensorFlow Lite initialized!");
}

// Function to run inference with the model
float runInference(float spo2, float bpm, float tempF) {
  // Load data into the input tensor
  tflInputTensor->data.f[0] = spo2;   // SpO2 value
  tflInputTensor->data.f[1] = bpm;    // BPM value
  tflInputTensor->data.f[2] = tempF;  // Temperature value

  // Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Failed to invoke the model!");
    return -1; // Return an error value
  }

  // Get the output tensor
  float* output = tflOutputTensor->data.f;

  // Return the first output (adjust based on your model)
  return output[0];
}

#endif  // MODEL_HELPERS_H
