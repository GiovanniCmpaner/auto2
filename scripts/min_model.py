import tensorflow as tf

input = tf.keras.Input(shape=(7,))
output = tf.keras.layers.Dense(16, activation='relu')(input)
output = tf.keras.layers.Dense(5, activation='relu')(output)
model = tf.keras.Model(inputs=input, outputs=output)

model.compile(optimizer='adam', loss='mse', metrics=['mae'])

model.save('model_path', save_format='tf')