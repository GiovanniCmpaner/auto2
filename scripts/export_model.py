# -------------------------------------------------------------------------------
# Configure Defaults
# -------------------------------------------------------------------------------

# Define paths to model files
import os
MODELS_DIR = 'models/'
if not os.path.exists(MODELS_DIR): os.mkdir(MODELS_DIR)
MODEL_TF = MODELS_DIR + 'model'
MODEL_NO_QUANT_TFLITE = MODELS_DIR + 'model_no_quant.tflite'
MODEL_TFLITE = MODELS_DIR + 'model.tflite'
MODEL_TFLITE_MICRO = MODELS_DIR + 'model.cc'

# -------------------------------------------------------------------------------
# Setup Environment
# -------------------------------------------------------------------------------

# TensorFlow is an open source machine learning library
import tensorflow as tf

# Keras is TensorFlow's high-level API for deep learning
from tensorflow import keras
# Numpy is a math library
import numpy as np
# Pandas is a data manipulation library 
import pandas as pd
# Matplotlib is a graphing library
import matplotlib.pyplot as plt
# Math is Python's math library
import math

# Set seed for experiment reproducibility
seed = 1
np.random.seed(seed)
tf.random.set_seed(seed)

#fig, ax = plt.subplots(3, 3)
fig = plt.figure()
fig.subplots_adjust(left=0.06, bottom=0.03, right=0.97, top=0.97, wspace=0.25, hspace=0.4)

# -------------------------------------------------------------------------------
# Generate the Data
# -------------------------------------------------------------------------------

# Number of sample datapoints
SAMPLES = 1000

# Generate a uniformly distributed set of random numbers in the range from
# 0 to 2π, which covers a complete sine wave oscillation
x_values = np.random.uniform(low=0, high=2*math.pi, size=SAMPLES).astype(np.float32)

# Shuffle the values to guarantee they're not in order
np.random.shuffle(x_values)

# Calculate the corresponding sine values
y_values = np.sin(x_values).astype(np.float32)

# Plot our data. The 'b.' argument tells the library to print blue dots.
plt.subplot(3, 3, 1)
plt.plot(x_values, y_values, 'b.')
plt.title('Perfect data')

# Add a small random number to each y value
y_values += 0.1 * np.random.randn(*y_values.shape)

# Plot our data
plt.subplot(3, 3, 2)
plt.plot(x_values, y_values, 'b.')
plt.title('Data with noise')

# -------------------------------------------------------------------------------
# Split the Data
# -------------------------------------------------------------------------------

# We'll use 60% of our data for training and 20% for testing. The remaining 20%
# will be used for validation. Calculate the indices of each section.
TRAIN_SPLIT =  int(0.6 * SAMPLES)
TEST_SPLIT = int(0.2 * SAMPLES + TRAIN_SPLIT)

# Use np.split to chop our data into three parts.
# The second argument to np.split is an array of indices where the data will be
# split. We provide two indices, so the data will be divided into three chunks.
x_train, x_test, x_validate = np.split(x_values, [TRAIN_SPLIT, TEST_SPLIT])
y_train, y_test, y_validate = np.split(y_values, [TRAIN_SPLIT, TEST_SPLIT])

# Double check that our splits add up correctly
assert (x_train.size + x_validate.size + x_test.size) ==  SAMPLES

# Plot the data in each partition in different colors:
plt.subplot(3, 3, 3)
plt.plot(x_train, y_train, 'b.', label="Train")
plt.plot(x_test, y_test, 'r.', label="Test")
plt.plot(x_validate, y_validate, 'y.', label="Validate")
plt.title('Split data')
plt.legend()

# -------------------------------------------------------------------------------
# Design the Model
# -------------------------------------------------------------------------------

# We'll use Keras to create a simple model architecture
model = tf.keras.Sequential()

# First layer takes a scalar input and feeds it through 16 "neurons". The
# neurons decide whether to activate based on the 'relu' activation function.
model.add(keras.layers.Dense(16, activation='relu', input_shape=(1,)))

# The new second and third layer will help the network learn more complex representations
model.add(keras.layers.Dense(16, activation='relu'))

# Final layer is a single neuron, since we want to output a single value
model.add(keras.layers.Dense(1))

# Compile the model using the standard 'adam' optimizer and the mean squared error or 'mse' loss function for regression.
model.compile(optimizer='adam', loss='mse', metrics=['mae'])

# -------------------------------------------------------------------------------
# Train the Model
# -------------------------------------------------------------------------------

# Train the model on our training data while validating on our validation set
history = model.fit(x_train, y_train, epochs=500, batch_size=64, validation_data=(x_validate, y_validate))

# Save the model to disk
model.save(MODEL_TF)

# -------------------------------------------------------------------------------
# Plot Metrics - Loss (or Mean Squared Error)
# -------------------------------------------------------------------------------

# Draw a graph of the loss, which is the distance between
# the predicted and actual values during training and validation.
train_loss = history.history['loss']
val_loss = history.history['val_loss']

epochs = range(1, len(train_loss) + 1)

# Exclude the first few epochs so the graph is easier to read
SKIP = 50

plt.subplot(3, 3, 4)
plt.title('Training and validation loss')
plt.plot(epochs[SKIP:], train_loss[SKIP:], 'g.', label='Training loss')
plt.plot(epochs[SKIP:], val_loss[SKIP:], 'b.', label='Validation loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()

# -------------------------------------------------------------------------------
# Plot Metrics - Mean Absolute Error
# -------------------------------------------------------------------------------

# Draw a graph of mean absolute error, which is another way of
# measuring the amount of error in the prediction.
train_mae = history.history['mae']
val_mae = history.history['val_mae']

plt.subplot(3, 3, 5)
plt.title('Training and validation mean absolute error')
plt.plot(epochs[SKIP:], train_mae[SKIP:], 'g.', label='Training MAE')
plt.plot(epochs[SKIP:], val_mae[SKIP:], 'b.', label='Validation MAE')
plt.xlabel('Epochs')
plt.ylabel('MAE')
plt.legend()

# -------------------------------------------------------------------------------
# Plot Metrics - Actual vs Predicted Outputs
# -------------------------------------------------------------------------------

# Calculate and print the loss on our test dataset
test_loss, test_mae = model.evaluate(x_test, y_test)

# Make predictions based on our test dataset
y_test_pred = model.predict(x_test)

# Graph the predictions against the actual values
plt.subplot(3, 3, 6)
plt.title('Comparison of predictions and actual values')
plt.plot(x_test, y_test, 'b.', label='Actual values')
plt.plot(x_test, y_test_pred, 'r.', label='TF predictions')
plt.legend()

# -------------------------------------------------------------------------------
# Generate a TensorFlow Lite Model - With or Without Quantization
# -------------------------------------------------------------------------------

# Convert the model to the TensorFlow Lite format without quantization
converter = tf.lite.TFLiteConverter.from_saved_model(MODEL_TF)
model_no_quant_tflite = converter.convert()

# Save the model to disk
open(MODEL_NO_QUANT_TFLITE, "wb").write(model_no_quant_tflite)

# Convert the model to the TensorFlow Lite format with quantization
def representative_dataset():
  for i in range(500):
    yield([x_train[i].reshape(1, 1)])
# Set the optimization flag.
converter.optimizations = [tf.lite.Optimize.DEFAULT]
# Enforce integer only quantization
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
# Provide a representative dataset to ensure we quantize correctly.
converter.representative_dataset = representative_dataset
model_tflite = converter.convert()

# Save the model to disk
open(MODEL_TFLITE, "wb").write(model_tflite)

# -------------------------------------------------------------------------------
# Compare Model Performance - Helper functions
# -------------------------------------------------------------------------------

def predict_tflite(tflite_model, x_test):
  # Prepare the test data
  x_test_ = x_test.copy()
  x_test_ = x_test_.reshape((x_test.size, 1))
  x_test_ = x_test_.astype(np.float32)

  # Initialize the TFLite interpreter
  interpreter = tf.lite.Interpreter(model_content=tflite_model)
  interpreter.allocate_tensors()

  input_details = interpreter.get_input_details()[0]
  output_details = interpreter.get_output_details()[0]

  # If required, quantize the input layer (from float to integer)
  input_scale, input_zero_point = input_details["quantization"]
  if (input_scale, input_zero_point) != (0.0, 0):
    x_test_ = x_test_ / input_scale + input_zero_point
    x_test_ = x_test_.astype(input_details["dtype"])
  
  # Invoke the interpreter
  y_pred = np.empty(x_test_.size, dtype=output_details["dtype"])
  for i in range(len(x_test_)):
    interpreter.set_tensor(input_details["index"], [x_test_[i]])
    interpreter.invoke()
    y_pred[i] = interpreter.get_tensor(output_details["index"])[0]
  
  # If required, dequantized the output layer (from integer to float)
  output_scale, output_zero_point = output_details["quantization"]
  if (output_scale, output_zero_point) != (0.0, 0):
    y_pred = y_pred.astype(np.float32)
    y_pred = (y_pred - output_zero_point) * output_scale

  return y_pred

def evaluate_tflite(tflite_model, x_test, y_true):
  global model
  y_pred = predict_tflite(tflite_model, x_test)
  loss_function = tf.keras.losses.get(model.loss)
  loss = loss_function(y_true, y_pred).numpy()
  return loss
  
# -------------------------------------------------------------------------------
# Compare Model Performance - Predictions
# -------------------------------------------------------------------------------

# Calculate predictions
y_test_pred_tf = model.predict(x_test)
y_test_pred_no_quant_tflite = predict_tflite(model_no_quant_tflite, x_test)
y_test_pred_tflite = predict_tflite(model_tflite, x_test)

# Compare predictions
plt.subplot(3, 3, 7)
plt.title('Comparison of various models against actual values')
plt.plot(x_test, y_test, 'bo', label='Actual values')
plt.plot(x_test, y_test_pred_tf, 'ro', label='TF predictions')
plt.plot(x_test, y_test_pred_no_quant_tflite, 'bx', label='TFLite predictions')
plt.plot(x_test, y_test_pred_tflite, 'gx', label='TFLite quantized predictions')
plt.legend()

# -------------------------------------------------------------------------------
# Compare Model Performance - Loss (MSE/Mean Squared Error)
# -------------------------------------------------------------------------------

# Calculate loss
x_loss = [
    "TensorFlow", 
    "TensorFlow Lite", 
    "TensorFlow Lite Quantized"
]
y_loss = [ 
    model.evaluate(x_test, y_test, verbose=0)[0], 
    evaluate_tflite(model_no_quant_tflite, x_test, y_test), 
    evaluate_tflite(model_tflite, x_test, y_test)
]

# Compare loss
plt.subplot(3, 3, 8)
plt.title('Comparison of loss between models')
plt.bar(x_loss, y_loss)
plt.xlabel('Model')
plt.ylabel('Loss/MSE')

y_loss_min = min(y_loss)
for i in range(len(y_loss)):
    percentage = int(y_loss_min/y_loss[i] * 100)
    plt.annotate(f"{percentage}%", xy=(x_loss[i],y_loss[i]), ha='center', va='bottom')

# -------------------------------------------------------------------------------
# Compare Model Performance - Size
# -------------------------------------------------------------------------------

# Calculate size
x_size = [
    "TensorFlow", 
    "TensorFlow Lite", 
    "TensorFlow Lite Quantized"
]
y_size = [ 
    os.path.getsize(MODEL_TF), 
    os.path.getsize(MODEL_NO_QUANT_TFLITE), 
    os.path.getsize(MODEL_TFLITE) 
]

plt.subplot(3, 3, 9)
plt.title('Comparison of size between models')
plt.bar(x_size, y_size)
plt.xlabel('Model')
plt.ylabel('Bytes')

y_size_max = max(y_size)
for i in range(len(y_size)):
    percentage = int(y_size[i]/y_size_max * 100)
    plt.annotate(f"{percentage}%", xy=(x_size[i],y_size[i]), ha='center', va='bottom')

# -------------------------------------------------------------------------------
# Generate a TensorFlow Lite for Microcontrollers Model
# -------------------------------------------------------------------------------

# Convert to a C source file, i.e, a TensorFlow Lite for Microcontrollers model
os.system(f"xxd -i {MODEL_TFLITE} > {MODEL_TFLITE_MICRO}")

# -------------------------------------------------------------------------------
# Plot
# -------------------------------------------------------------------------------

mng = plt.get_current_fig_manager()
mng.window.state('zoomed')
plt.pause(0.05)
plt.show()

# -------------------------------------------------------------------------------
# End
# -------------------------------------------------------------------------------