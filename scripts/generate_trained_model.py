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
from tensorflow.python.framework import graph_util
from tensorflow.python.framework import graph_io

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

import inspect

# Set seed for experiment reproducibility
seed = 1
np.random.seed(seed)
tf.random.set_seed(seed)

# configure gpu
physical_devices = tf.config.list_physical_devices('GPU') 
tf.config.experimental.set_memory_growth(physical_devices[0], True)

# Subplotting options and layout
fig = plt.figure()
fig.subplots_adjust(left=0.06, bottom=0.03, right=0.97, top=0.97, wspace=0.25, hspace=0.4)

# -------------------------------------------------------------------------------
# Load the Data
# -------------------------------------------------------------------------------

# Get file data
df_features = pd.read_csv("features.csv", sep=";")
df_labels = pd.read_csv("labels.csv", sep=";")

dataset = tf.data.Dataset.from_tensor_slices((df_features.values, df_labels.values))

# Plot our data
#plt.subplot(3, 3, 1)
#plt.plot(df_features["f0"], df_labels.values + 0.00, marker="x", c="red", label="f0")
#plt.plot(df_features["f1"], df_labels.values + 0.05, marker="x", c="green", label="f1")
#plt.plot(df_features["f2"], df_labels.values + 0.10, marker="x", c="blue", label="f2")
#plt.plot(df_features["f3"], df_labels.values + 0.15, marker="x", c="magenta", label="f3")
#plt.plot(df_features["f4"], df_labels.values + 0.20, marker="x", c="yellow", label="f4")
#plt.plot(df_features["f5"], df_labels.values + 0.25, marker="x", c="cyan", label="f5")
#plt.plot(df_features["f6"], df_labels.values + 0.30, marker="x", c="orange", label="f6")
#plt.title('Data')
#plt.legend()

# -------------------------------------------------------------------------------
# Split the Data
# -------------------------------------------------------------------------------

# Number of sample datapoints
SAMPLES = len(df_features.values)

# We'll use 60% of our data for training and 20% for testing. The remaining 20%
# will be used for validation. Calculate the indices of each section.
TRAIN_SPLIT =  int(0.7 * SAMPLES)
TEST_SPLIT = int(0.3 * SAMPLES + TRAIN_SPLIT)

# Use np.split to chop our data into three parts.
# The second argument to np.split is an array of indices where the data will be
# split. We provide two indices, so the data will be divided into three chunks.
x_train, x_test, x_validate = np.vsplit(df_features.values, [TRAIN_SPLIT, TEST_SPLIT])
y_train, y_test, y_validate = np.vsplit(df_labels.values, [TRAIN_SPLIT, TEST_SPLIT])

# Double check that our splits add up correctly
assert (len(x_train) + len(x_test) + len(x_validate)) ==  SAMPLES

# -------------------------------------------------------------------------------
# Design the Model
# -------------------------------------------------------------------------------

# We'll use Keras to create a simple model architecture
model = tf.keras.Sequential([
    keras.layers.Dense(8, activation='relu'),
    keras.layers.Dense(16, activation='relu'),
    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dense(16, activation='relu'),
    keras.layers.Dense(5),
])

# Compile the model using the standard 'adam' optimizer and the mean squared error or 'mse' loss function for regression.
model.compile(optimizer='adam', loss='mse', metrics=['categorical_accuracy'])

# -------------------------------------------------------------------------------
# Train the Model
# -------------------------------------------------------------------------------



options = tf.saved_model.SaveOptions(save_debug_info=True, experimental_variable_policy=tf.saved_model.experimental.VariablePolicy.SAVE_VARIABLE_DEVICES)



# Train the model on our training data while validating on our validation set
history = model.fit(x_train, y_train, epochs=20, batch_size=1024, shuffle=True, validation_data=(x_validate, y_validate))

# Save the model to disk
model.save(MODEL_TF, save_format='tf')

# Converting a SavedModel to a TensorFlow Lite model.
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save the model to disk
open(MODEL_NO_QUANT_TFLITE, "wb").write(tflite_model)

quit()
# -------------------------------------------------------------------------------
# Plot Metrics - Loss (or Mean Squared Error)
# -------------------------------------------------------------------------------

# Draw a graph of the loss, which is the distance between
# the predicted and actual values during training and validation.
train_loss = history.history['loss']
val_loss = history.history['val_loss']

epochs = range(1, len(train_loss) + 1)

# Exclude the first few epochs so the graph is easier to read
SKIP = 0

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
# Compare Model Performance - Helper functions
# -------------------------------------------------------------------------------

def predict_tflite(tflite_model, x_test):
  # Prepare the test data
  x_test_ = x_test.copy().astype(np.float32)

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
  y_pred = np.empty(shape=y_test.shape, dtype=output_details["dtype"])
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
  loss = loss_function(y_true, y_pred).numpy().mean()
  return loss
  
# -------------------------------------------------------------------------------
# Compare Model Performance - Predictions
# -------------------------------------------------------------------------------

# Calculate predictions
#y_test_pred_tf = model.predict(x_test)
#y_test_pred_no_quant_tflite = predict_tflite(model_no_quant_tflite, x_test)
#y_test_pred_tflite = predict_tflite(model_tflite, x_test)

# Compare predictions
#plt.subplot(3, 3, 7)
#plt.title('Comparison of various models against actual values')
#plt.plot(x_test, y_test, 'bo', label='Actual values')
#plt.plot(x_test, y_test_pred_tf, 'ro', label='TF predictions')
#plt.plot(x_test, y_test_pred_no_quant_tflite, 'bx', label='TFLite predictions')
#plt.plot(x_test, y_test_pred_tflite, 'gx', label='TFLite quantized predictions')
#plt.legend()

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

# Draw percentage
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

# Compare size
plt.subplot(3, 3, 9)
plt.title('Comparison of size between models')
plt.bar(x_size, y_size)
plt.xlabel('Model')
plt.ylabel('Bytes')

# Draw percentage
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
plt.show()

# -------------------------------------------------------------------------------
# End
# -------------------------------------------------------------------------------