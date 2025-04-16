import tensorflow as tf
import subprocess
import os


# Load the .h5 model
model = tf.keras.models.load_model("model.h5")

# Convert the model to TensorFlow Lite format
converter = tf.lite.TFLiteConverter.from_keras_model(model)

# Optional: Enable quantization for optimization
converter.optimizations = [tf.lite.Optimize.DEFAULT]  # Uncomment this line to quantize the model

# Convert the model
tflite_model = converter.convert()

# Save the TensorFlow Lite model to a file
tflite_file = "model.tflite"
with open(tflite_file, "wb") as f:
    f.write(tflite_model)


tflite_size = os.path.getsize(tflite_file)

print(f"Model converted and saved as {tflite_file}")
print(f"Size of the TensorFlow Lite model: {tflite_size / 1024:.2f} KB")


# model conversion into .h file (using google colab) this is the code:

# !echo "const unsigned char model[] = {" > /content/model.h
# !cat model.tflite | xxd -i              >> /content/model.h
# !echo "};"                              >> /content/model.h

# import os
# model_h_size = os.path.getsize("model.h")
# print(f"Header file, model.h, is {model_h_size:,} bytes.")
# print("\nOpen the side panel (refresh if needed). Double click model.h to download the file.")