# -------------------------------------------------------------------------------
# Configure Defaults
# -------------------------------------------------------------------------------

# Define paths to model files
import os
MODELS_DIR = 'models/'
if not os.path.exists(MODELS_DIR): os.mkdir(MODELS_DIR)
MODEL_TF = MODELS_DIR + 'model'

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

from pathlib import Path

from timeit import default_timer as timer

# Set seed for experiment reproducibility
seed = 1
np.random.seed(seed)
tf.random.set_seed(seed)

# configure gpu
physical_devices = tf.config.list_physical_devices('GPU') 
tf.config.experimental.set_memory_growth(physical_devices[0], True)

# Subplotting options and layout
fig = plt.figure(figsize=(20,5), dpi=96)
#fig.subplots_adjust(left=0.05, bottom=0.2, right=0.95, top=0.9, wspace=0.25, hspace=0.4)

# Show charts as full window
#mng = plt.get_current_fig_manager()
#mng.window.state('zoomed')

def train_model(input_file, combination):
    # -------------------------------------------------------------------------------
    # Load the Data
    # -------------------------------------------------------------------------------

    # Get file data
    df_capture = pd.read_csv(input_file, sep=";")
    df_features, df_labels = df_capture.iloc[:,0:6], df_capture.iloc[:,6:11]

    dataset = tf.data.Dataset.from_tensor_slices((df_features.values, df_labels.values))
    
    labels = []
    for row in df_labels.values:
        for i in range(0,5):
            if(row[i] == 1):
                labels.append(i)
    
    plt.subplot(1,1,1)
    plt.title('Graphic')
    plt.scatter(labels, df_features.values[:,0], label='1')
    plt.scatter(labels, df_features.values[:,1], label='2')
    plt.scatter(labels, df_features.values[:,2], label='3')
    plt.scatter(labels, df_features.values[:,3], label='4')
    plt.scatter(labels, df_features.values[:,4], label='5')
    plt.scatter(labels, df_features.values[:,5], label='6')
    plt.xlabel('Epochs')
    plt.ylabel('MAE')
    plt.legend()
    plt.show()
    quit()

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
    model = tf.keras.Sequential()
    model.add(keras.layers.Dense(6))
    for layer in combination:
        model.add(keras.layers.Dense(layer, activation='relu'))
    model.add(keras.layers.Dense(5, activation='softmax'))

    # Compile the model using the standard 'adam' optimizer and the mean squared error or 'mse' loss function for regression.
    model.compile(optimizer=tf.keras.optimizers.SGD(learning_rate=0.05, momentum=0.8), loss='categorical_crossentropy', metrics=['categorical_accuracy'])

    # -------------------------------------------------------------------------------
    # Train the Model
    # -------------------------------------------------------------------------------

    options = tf.saved_model.SaveOptions(save_debug_info=True, experimental_variable_policy=tf.saved_model.experimental.VariablePolicy.SAVE_VARIABLE_DEVICES)

    start = timer()

    # Train the model on our training data while validating on our validation set
    history = model.fit(x_train, y_train, epochs=50, batch_size=64, shuffle=True, validation_data=(x_validate, y_validate))
    loss = history.history['loss']
    accuracy = history.history['categorical_accuracy']
    
    end = timer()
    
    # Get training interval
    interval = end - start

    # Save the model to disk
    model.save(MODEL_TF, save_format='tf')

    # Test some data
    x_test1 = [[0.378, 0.154, 0.283, 0.184, 0.162, 0.123]]
    y_pred1 = model.predict(x_test1)
    print('x_test1 = ' + str(x_test1) + ' y_pred1 = ' + str(y_pred1))

    x_test2 = [[0.363, 0.773, 0.245, 0.150, 0.208, 0.170]]
    y_pred2 = model.predict(x_test2)
    print('x_test2 = ' + str(x_test2) + ' y_pred2 = ' + str(y_pred2))

    # Converting a SavedModel to a TensorFlow Lite model.
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()

    # Save the model to disk
    file_path = MODELS_DIR + Path(input_file).stem + '_' + 'x'.join(map(str,combination)) + '_model_no_quant.tflite'
    file_size = open(file_path, "wb").write(tflite_model)
    
    return file_size, interval, loss, accuracy
    
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
    
    return

def train_by_combination():

    #combinations = [[8],[16],[32],[16,16],[32,32],[48,48],[32,16,32],[48,32,48],[64,48,64]]
    #courses = [5, 10, 15, 20, 25, 30, 35, 40]

    combinations = [[16,16]]
    courses = [5, 10, 15, 20, 25, 30, 35, 40]
    epochs = [1, 10, 20, 30, 40, 50]

    for i, combination in enumerate(combinations):

        sizes = []
        intervals = []
        losses = []
        accuracies = []

        for j, course in enumerate(courses):
            size, interval, loss, accuracy = train_model(r"D:\Google Drive\TCC SENAI\Capturas\capture_" + str(course) + "x_1,5x1m.csv", combination)
            sizes.append(size)
            intervals.append(interval)
            losses.append(loss)
            accuracies.append(accuracy)
        
        plt.subplot(len(combinations), 4, 4 * i + 1)
        plt.title('Tamanhos')
        plt.plot(courses, sizes, linestyle='solid', marker='.', label=str(epochs[-1]))
        plt.xlabel('Percursos')
        plt.ylabel('Bytes')
        plt.legend(title="Épocas")
        
        plt.subplot(len(combinations), 4, 4 * i + 2)
        plt.title('Durações')
        plt.plot(courses, intervals, linestyle='solid', marker='.', label=str(epochs[-1]))
        plt.xlabel('Percursos')
        plt.ylabel('Segundos')
        plt.legend(title="Épocas")

        plt.subplot(len(combinations), 4, 4 * i + 3)
        plt.title('Perdas')
        for j, epoch in enumerate(epochs):
            plt.plot(courses, np.array(losses)[:,epoch - 1].tolist(), linestyle='solid', marker='.', label=str(epoch))
        plt.xlabel('Percursos')
        plt.ylabel('Perda')
        plt.legend(title="Épocas")

        plt.subplot(len(combinations), 4, 4 * i + 4)
        plt.title('Precisões')
        for j, epoch in enumerate(epochs):
            plt.plot(courses, np.array(accuracies)[:,epoch - 1].tolist(), linestyle='solid', marker='.', label=str(epoch))
        plt.xlabel('Percursos')
        plt.ylabel('Precisão')
        plt.legend(title="Épocas")
        
        plt.tight_layout()
        plt.savefig(r"C:\Users\Giovanni\Desktop\auto2\scripts\models\graficos_percursos_16x16combinacao.png")
        plt.clf()
    
def train_by_best():
    
    combinations = [[16],[32],[48],[16,16],[32,32],[48,48],[16,16,16],[32,32,32],[48,48,48]]
    
    sizes = []
    intervals = []
    losses = []
    accuracies = []
    
    for i, combination in enumerate(combinations):
        size, interval, loss, accuracy = train_model(r"D:\Google Drive\TCC SENAI\Capturas\capture_40x_1,5x1m.csv", combination)
        sizes.append(size)
        intervals.append(interval)
        losses.append(loss[-1])
        accuracies.append(accuracy[-1])

    plt.subplot(1, 4, 1)
    plt.title('Tamanhos')
    plt.plot(list(map(lambda x: 'x'.join(map(str,x)), combinations)), sizes, linestyle='solid', marker='.')
    plt.xticks(rotation=45, ha='right')
    plt.xlabel('Combinação')
    plt.ylabel('Bytes')

    plt.subplot(1, 4, 2)
    plt.title('Durações')
    plt.plot(list(map(lambda x: 'x'.join(map(str,x)), combinations)), intervals, linestyle='solid', marker='.')
    plt.xticks(rotation=45, ha='right')
    plt.xlabel('Combinação')
    plt.ylabel('Segundos')
     
    plt.subplot(1, 4, 3)
    plt.title('Perdas')
    plt.plot(list(map(lambda x: 'x'.join(map(str,x)), combinations)), losses, linestyle='solid', marker='.')
    plt.xticks(rotation=45, ha='right')
    plt.xlabel('Combinação')
    plt.ylabel('Perda')
    
    plt.subplot(1, 4, 4)
    plt.title('Precisões')
    plt.plot(list(map(lambda x: 'x'.join(map(str,x)), combinations)), accuracies, linestyle='solid', marker='.')
    plt.xticks(rotation=45, ha='right')
    plt.xlabel('Combinação')
    plt.ylabel('Precisão')
    
    plt.tight_layout()
    plt.savefig(r"C:\Users\Giovanni\Desktop\auto2\scripts\models\graficos_combinacoes_40percursos_50epocas.png")
    plt.clf()
     
train_by_best()
train_by_combination()

# -------------------------------------------------------------------------------
# End
# -------------------------------------------------------------------------------