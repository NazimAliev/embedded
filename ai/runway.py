#!/usr/bin/python3
# coding: utf-8

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#
# Synthetic helicopter and jets images recognition
# App based on MNIST Google Colab app
#

#!pip install -U --pre tensorflow=="2.*"
import os, signal, re, time, json
import PIL.Image, PIL.ImageFont, PIL.ImageDraw
import numpy as np
import tensorflow as tf
import tensorflow_datasets as tfds
from matplotlib import pyplot as plt
print("Tensorflow version " + tf.__version__)
#print(tfds.list_builders())

"""## Autorization and Cloud parms"""

IS_COLAB_BACKEND = 'COLAB_GPU' in os.environ  # this is always set on Colab, the value is 0 or 1 depending on GPU presence
if IS_COLAB_BACKEND:
  print("Auth now:")
  from google.colab import auth
  # Authenticates the Colab machine and also the TPU using your
  # credentials so that they can access your private GCS buckets.
  auth.authenticate_user()

PROJECT = "neuralstaff" #@param {type:"string"}
BUCKET = "gs://neuralstaff-bucket/"  #@param {type:"string", default:"jddj"}
NEW_MODEL = True #@param {type:"boolean"}
MODEL_NAME = "runway" #@param {type:"string"}
MODEL_VERSION = "v1" #@param {type:"string"}

assert PROJECT, 'For this part, you need a GCP project. Head to http://console.cloud.google.com/ and create one.'
assert re.search(r'gs://.+', BUCKET), 'For this part, you need a GCS bucket. Head to http://console.cloud.google.com/storage and create one.'

ISIZE = 20000
# items in  training dataset
# so BATCH_SIZE = ITEMS then ITEMS should be factor of 8

VSIZE = 10000
# items in validation dataset

IM_SIZE = 128
# size of image

"""## Helper functions"""

#@title visualization utilities [RUN ME]
"""
This cell contains helper functions used for visualization
and downloads only. You can skip reading it. There is very
little useful Keras/Tensorflow code here.
"""

# Matplotlib config
plt.rc('image', cmap='gray_r')
plt.rc('grid', linewidth=0)
plt.rc('xtick', top=False, bottom=False, labelsize='large')
plt.rc('ytick', left=False, right=False, labelsize='large')
plt.rc('axes', facecolor='F8F8F8', titlesize="large", edgecolor='white')
plt.rc('text', color='a8151a')
plt.rc('figure', facecolor='F0F0F0')# Matplotlib fonts
MATPLOTLIB_FONT_DIR = os.path.join(os.path.dirname(plt.__file__), "mpl-data/fonts/ttf")

# pull a batch from the datasets. This code is not very nice, it gets much better in eager mode (TODO)
def dataset_to_numpy_util(training_dataset, validation_dataset, N):
  
  # get one batch from each: 10000 validation images, N training images
  batch_train_ds = training_dataset.unbatch().batch(N)
  
  # eager execution: loop through datasets normally
  if tf.executing_eagerly():
    for validation_images, validation_labels in validation_dataset:
      validation_images = validation_images.numpy()
      validation_labels = validation_labels.numpy()
      break
    for training_images, training_labels in batch_train_ds:
      training_images = training_images.numpy()
      training_labels = training_labels.numpy()
      break
    
  else:
    v_images, v_labels = tf.compat.v1.data.make_one_shot_iterator(validation_dataset).get_next()
    t_images, t_labels = tf.compat.v1.data.make_one_shot_iterator(batch_train_ds).get_next()
    # Run once, get one batch. Session.run returns numpy results
    with tf.Session() as ses:
      (validation_images, validation_labels,
       training_images, training_labels) = ses.run([v_images, v_labels, t_images, t_labels])
  
  # these were one-hot encoded in the dataset
  validation_labels = np.argmax(validation_labels, axis=1)
  training_labels = np.argmax(training_labels, axis=1)
  
  return (training_images, training_labels,
          validation_images, validation_labels)

# create images from local fonts for testing
def create_images_from_local_fonts(n):
  font_labels = []
  img = PIL.Image.new('LA', (IM_SIZE*n, IM_SIZE), color = (0,255)) # format 'LA': black in channel 0, alpha in channel 1
  font1 = PIL.ImageFont.truetype(os.path.join(MATPLOTLIB_FONT_DIR, 'DejaVuSansMono-Oblique.ttf'), 25)
  font2 = PIL.ImageFont.truetype(os.path.join(MATPLOTLIB_FONT_DIR, 'STIXGeneral.ttf'), 25)
  d = PIL.ImageDraw.Draw(img)
  for i in range(n):
    font_labels.append(i%10)
    d.text((7+i*IM_SIZE,0 if i<10 else -4), str(i%10), fill=(255,255), font=font1 if i<10 else font2)
  font_images = np.array(img.getdata(), np.float32)[:,0] / 255.0 # black in channel 0, alpha in channel 1 (discarded)
  font_images = np.reshape(np.stack(np.split(np.reshape(font_images, [IM_SIZE, IM_SIZE*n]), n, axis=1), axis=0), [n, 28*28])
  return font_images, font_labels

# utility to display a row of images with their predictions
def display_images(images, predictions, labels, title, n):
  plt.figure(figsize=(13,3))
  images = np.reshape(images, [n, IM_SIZE, IM_SIZE])
  images = np.swapaxes(images, 0, 1)
  images = np.reshape(images, [IM_SIZE, IM_SIZE*n])
  plt.yticks([])
  plt.xticks([IM_SIZE*x+14 for x in range(n)], predictions)
  for i,t in enumerate(plt.gca().xaxis.get_ticklabels()):
    if predictions[i] != labels[i]: t.set_color('red') # bad predictions in red
  plt.imshow(images)
  plt.grid(None)
  plt.title(title)
  
# utility to display multiple rows of images, sorted by unrecognized/recognized status
def display_top_unrecognized(images, predictions, labels, n, lines):
  idx = np.argsort(predictions==labels) # sort order: unrecognized first
  for i in range(lines):
    display_images(images[idx][i*n:(i+1)*n], predictions[idx][i*n:(i+1)*n], labels[idx][i*n:(i+1)*n],
                   "{} sample validation images out of {} with bad predictions in red and sorted first".format(n*lines, len(images)) if i==0 else "", n)
    
# utility to display training and validation curves
def display_training_curves(training, validation, title, subplot):
  if subplot%10==1: # set up the subplots on the first call
    plt.subplots(figsize=(10,10), facecolor='#F0F0F0')
    plt.tight_layout()
  ax = plt.subplot(subplot)
  ax.grid(linewidth=1, color='white')
  ax.plot(training)
  ax.plot(validation)
  ax.set_title('model '+ title)
  ax.set_ylabel(title)
  ax.set_xlabel('epoch')
  ax.legend(['train', 'valid.'])

"""## Detect hardware"""

# Detect hardware
try:
  tpu = tf.distribute.cluster_resolver.TPUClusterResolver() # TPU detection
except ValueError:
  tpu = None
  gpus = tf.config.experimental.list_logical_devices("GPU")
    
# Select appropriate distribution strategy
if tpu:
  tf.tpu.experimental.initialize_tpu_system(tpu)
  strategy = tf.distribute.experimental.TPUStrategy(tpu, steps_per_run=128) # Going back and forth between TPU and host is expensive. Better to run 128 batches on the TPU before reporting back.
  print('Running on TPU ', tpu.cluster_spec().as_dict()['worker'])  
elif len(gpus) > 1:
  strategy = tf.distribute.MirroredStrategy([gpu.name for gpu in gpus])
  print('Running on multiple GPUs ', [gpu.name for gpu in gpus])
elif len(gpus) == 1:
  strategy = tf.distribute.get_strategy() # default strategy that works on CPU and single GPU
  print('Running on single GPU ', gpus[0].name)
else:
  strategy = tf.distribute.get_strategy() # default strategy that works on CPU and single GPU
  print('Running on CPU')
print("Number of accelerators: ", strategy.num_replicas_in_sync)

"""## Parameters"""

BATCH_SIZE = 64 * strategy.num_replicas_in_sync # Gobal batch size.
print(strategy.num_replicas_in_sync)
#BATCH_SIZE = ISIZE // 4
# The global batch size will be automatically sharded across all
# replicas by the tf.data.Dataset API. A single TPU has 8 cores.
# The best practice is to scale the batch size by the number of
# replicas (cores). The learning rate should be increased as well.

LEARNING_RATE = 0.01
LEARNING_RATE_EXP_DECAY = 0.6 if strategy.num_replicas_in_sync == 1 else 0.7
# Learning rate computed later as LEARNING_RATE * LEARNING_RATE_EXP_DECAY**epoch
# 0.7 decay instead of 0.6 means a slower decay, i.e. a faster learnign rate.

training_images_file   = BUCKET + 'training-images-file'
training_labels_file   = BUCKET + 'training-labels-file'
validation_images_file = BUCKET + 'validation-images-file'
validation_labels_file = BUCKET + 'validation-labels-file'

!gsutil ls gs://neuralstaff-bucket
#!gsutil cat gs://neuralstaff-bucket/training-images-file > /tmp/image
#x = np.fromfile("/tmp/image")[:IM_SIZE*IM_SIZE]
#print(x.shape)
#print(BATCH_SIZE)

"""## Parse helper functions"""

def read_label(tf_bytestring):
    label = tf.io.decode_raw(tf_bytestring, tf.uint8)
    label = tf.reshape(label, [])
    label = tf.one_hot(label, 2)
    return label
  
def read_image(tf_bytestring):
    image = tf.io.decode_raw(tf_bytestring, tf.uint8)
    image = tf.cast(image, tf.float32)/255.0
    image = tf.reshape(image, [IM_SIZE*IM_SIZE])
    return image
  
def load_dataset(image_file, label_file):
    imagedataset = tf.data.FixedLengthRecordDataset(image_file, IM_SIZE*IM_SIZE, header_bytes=0)
    imagedataset = imagedataset.map(read_image, num_parallel_calls=16)
    labelsdataset = tf.data.FixedLengthRecordDataset(label_file, 1, header_bytes=0)
    labelsdataset = labelsdataset.map(read_label, num_parallel_calls=16)
    dataset = tf.data.Dataset.zip((imagedataset, labelsdataset))
    return dataset 
  
def get_training_dataset(image_file, label_file, batch_size):
    dataset = load_dataset(image_file, label_file)
    dataset = dataset.cache()  # this small dataset can be entirely cached in RAM
    dataset = dataset.shuffle(5000, reshuffle_each_iteration=True)
    dataset = dataset.repeat() # Mandatory for Keras for now
    dataset = dataset.batch(batch_size, drop_remainder=True) # drop_remainder is important on TPU, batch size must be fixed
    dataset = dataset.prefetch(-1)  # fetch next batches while training on the current one (-1: autotune prefetch buffer size)
    return dataset
  
def get_validation_dataset(image_file, label_file):
    dataset = load_dataset(image_file, label_file)
    dataset = dataset.cache() # this small dataset can be entirely cached in RAM
    dataset = dataset.batch(VSIZE, drop_remainder=True) # 10000 items in eval dataset, all in one batch
    dataset = dataset.repeat() # Mandatory for Keras for now
    return dataset

# instantiate the datasets
training_dataset = get_training_dataset(training_images_file, training_labels_file, BATCH_SIZE)
validation_dataset = get_validation_dataset(validation_images_file, validation_labels_file)

"""## Same play with input data"""

N = 20

(training_images, training_labels,
 validation_images, validation_labels) = dataset_to_numpy_util(training_dataset, validation_dataset, N)
print(training_images.shape, validation_images.shape)
display_images(training_images, training_labels, training_labels, "training images and their labels 1st part", N)
display_images(validation_images[:N], validation_labels[:N], validation_labels[:N], "validation images and their labels 1st part", N)
display_images(validation_images[VSIZE//2:VSIZE//2+N], validation_labels[VSIZE//2:VSIZE//2+N], validation_labels[VSIZE//2:VSIZE//2+N], "validation images and their labels 2nd part", N)

"""## Create model function"""

# This model trains to 99.4% accuracy in 10 epochs (with a batch size of 64)  

def make_model():
    model = tf.keras.Sequential(
      [
        tf.keras.layers.Reshape(input_shape=(IM_SIZE*IM_SIZE,), target_shape=(IM_SIZE, IM_SIZE, 1), name="image"),

        tf.keras.layers.Conv2D(filters=12, kernel_size=6, padding='same', use_bias=False), # no bias necessary before batch norm
        tf.keras.layers.BatchNormalization(scale=False, center=True), # no batch norm scaling necessary before "relu"
        tf.keras.layers.Activation('relu'), # activation after batch norm

        tf.keras.layers.Conv2D(filters=24, kernel_size=8, padding='same', use_bias=False, strides=2),
        tf.keras.layers.BatchNormalization(scale=False, center=True),
        tf.keras.layers.Activation('relu'),

        tf.keras.layers.Conv2D(filters=32, kernel_size=8, padding='same', use_bias=False, strides=2),
        tf.keras.layers.BatchNormalization(scale=False, center=True),
        tf.keras.layers.Activation('relu'),

        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(200, use_bias=False),
        tf.keras.layers.BatchNormalization(scale=False, center=True),
        tf.keras.layers.Activation('relu'),
        tf.keras.layers.Dropout(0.4), # Dropout on dense layer only
       
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dense(100, use_bias=False),
        tf.keras.layers.BatchNormalization(scale=False, center=True),
        tf.keras.layers.Activation('relu'),
        tf.keras.layers.Dropout(0.4), # Dropout on dense layer only

        tf.keras.layers.Dense(2, activation='softmax')
      ])

    model.compile(optimizer='adam', # learning rate will be set by LearningRateScheduler
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])
    return model

"""## Model parms output"""

with strategy.scope():
    model = make_model()

# print model layers
model.summary()

# set up learning rate decay
lr_decay = tf.keras.callbacks.LearningRateScheduler(
    lambda epoch: LEARNING_RATE * LEARNING_RATE_EXP_DECAY**epoch,
    verbose=True)

"""## Train and evaluate model"""

EPOCHS = 10
steps_per_epoch = ISIZE//BATCH_SIZE  # 60,000 items in this dataset
print("Steps per epoch: ", steps_per_epoch)
  
# Little wrinkle: in the present version of Tensorfow (1.14), switching a TPU
# between training and evaluation is slow (approx. 10 sec). For small models,
# it is recommeneded to run a single eval at the end.
history = model.fit(training_dataset,
                    steps_per_epoch=steps_per_epoch, epochs=EPOCHS,
                    callbacks=[lr_decay])

final_stats = model.evaluate(validation_dataset, steps=1)
print("Validation accuracy: ", final_stats[1])

"""## Show predictions"""

# recognize images from local fonts
#probabilities = model.predict(font_images, steps=1)
#predicted_labels = np.argmax(probabilities, axis=1)
#display_images(font_images, predicted_labels, font_labels, "predictions from local fonts (bad predictions in red)", N)
N = 20
# recognize validation images
probabilities = model.predict(validation_images, steps=1)
print(probabilities.shape)
predicted_labels = np.argmax(probabilities, axis=1)
display_images(validation_images[:N], predicted_labels[:N], validation_labels[:N], "predictions from training dataset 1st (bad predictions in red)", N)
display_images(validation_images[VSIZE//2:N+VSIZE//2], predicted_labels[VSIZE//2:N+VSIZE//2], validation_labels[VSIZE//2:N+VSIZE//2], "predictions from training dataset 2nd (bad predictions in red)", N)
display_top_unrecognized(validation_images[:N], predicted_labels[:N], validation_labels[:N], N, 1)
display_top_unrecognized(validation_images[VSIZE//2:N+VSIZE//2], predicted_labels[VSIZE//2:N+VSIZE//2], validation_labels[VSIZE//2:N+VSIZE//2], N, 1)

print(predicted_labels[:400])
print(predicted_labels[VSIZE//2:400+VSIZE//2])

"""## Save model"""

path = "/data/runway.h5" 
#model.save(path)
