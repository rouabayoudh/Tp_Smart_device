import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from keras.src.callbacks import EarlyStopping
from sklearn.preprocessing import StandardScaler
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix
from sklearn.metrics import classification_report, roc_auc_score
import seaborn as sns
from imblearn.over_sampling import SMOTE
from imblearn.under_sampling import RandomUnderSampler
from sklearn.utils.class_weight import compute_class_weight


# Load the dataset
df = pd.read_csv('Patient_Severity.csv')

# Select relevant features and target
X = df[['TEMPF', 'PULSE', 'POPCT']]  # temperature in F, pulse = heart rate, popct = oxygen
y = df['SCORE ']

# Handle missing values
df = df.dropna(subset=['TEMPF', 'PULSE', 'POPCT', 'SCORE '])

# Encode target labels
label_encoder = LabelEncoder()
y = label_encoder.fit_transform(y)

# One-hot encode the labels for multi-class classification
y_one_hot = tf.keras.utils.to_categorical(y, num_classes=len(label_encoder.classes_))

# Split data into train and test sets
X_train, X_test, y_train, y_test = train_test_split(X, y_one_hot, test_size=0.2, random_state=11)

# Scale the input features
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)


# Compute class weights (you can also manually define class weights)
class_weights = compute_class_weight('balanced', classes=np.unique(y), y=y)  # Use y instead of y_train

# The result is a dictionary with the class indices as keys
class_weights_dict = {i: class_weights[i] for i in range(len(class_weights))}

#tweaking
class_weights_dict[2] *= 2  # Increase weight for Class 2
class_weights_dict[3] *= 2.25  # Increase weight for Class 3


# ------------------- Oversampling with SMOTE -------------------
smote = SMOTE(random_state=42)
X_train_smote, y_train_smote = smote.fit_resample(X_train, y_train)

# ------------------- Undersampling with RandomUnderSampler -------------------
# undersampler = RandomUnderSampler(random_state=42)
# X_train_resampled, y_train_resampled = undersampler.fit_resample(X_train, y_train)

# # Choose the resampled dataset (comment/uncomment based on the technique you want to try)
X_train_resampled, y_train_resampled = X_train_smote, y_train_smote
# X_train_resampled, y_train_resampled = X_train_resampled, y_train_resampled



# Build the model
model = tf.keras.Sequential([
    tf.keras.layers.Dense(64, activation='relu', input_shape=(X_train_resampled.shape[1],)),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dense(32, activation='relu'),
    tf.keras.layers.BatchNormalization(),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(len(label_encoder.classes_), activation='softmax')  # Output layer for multi-class
])

# Learning rate schedule
initial_lr = 0.001
lr_schedule = tf.keras.optimizers.schedules.ExponentialDecay(
    initial_learning_rate=initial_lr,
    decay_steps=1000,  # Number of steps for decay
    decay_rate=0.9,    # Decay rate
    staircase=True     # If True, learning rate decreases in steps
)

focal_loss = tf.keras.losses.CategoricalFocalCrossentropy(
    alpha=0.25,         # Class balancing factor (adjust per class if needed)
    gamma=1.5,          # Higher gamma will focus on hard examples
    from_logits=False,   # If model outputs probabilities (softmax), set to False
    label_smoothing=0.0, # No smoothing
    axis=-1,             # Ensure it's applied on the last axis (for multi-class)
    reduction='sum_over_batch_size', # Sum losses across the batch, divide by batch size
    name='categorical_focal_crossentropy'
)

# Compile the model
optimizer = tf.keras.optimizers.Adam(learning_rate=lr_schedule)
model.compile(optimizer=optimizer, loss=focal_loss, metrics=['accuracy'])

# Implement early stopping
early_stopping = EarlyStopping(monitor='val_loss', patience=5, restore_best_weights=True)

# Train the model
history = model.fit(
    X_train, y_train,
    epochs=50,
    batch_size=32,
    validation_data=(X_test, y_test),
    callbacks=[early_stopping],
    class_weight=class_weights_dict
)

# Evaluate the model
test_loss, test_acc = model.evaluate(X_test, y_test)
print(f"Test Accuracy: {test_acc:.2f}")

# Predict the categories
predictions = model.predict(X_test)
predicted_labels = np.argmax(predictions, axis=1)  # Get the class with highest probability

# Convert predictions back to original labels
predicted_categories = label_encoder.inverse_transform(predicted_labels)
print(predicted_categories)

# Generate confusion matrix
conf_matrix = confusion_matrix(np.argmax(y_test, axis=1), predicted_labels)
print("Confusion Matrix:")
print(conf_matrix)

# Plot accuracy and loss curves
plt.figure(figsize=(12, 6))

# Accuracy
plt.subplot(1, 3, 1)
plt.plot(history.history['accuracy'], label='Train Accuracy')
plt.plot(history.history['val_accuracy'], label='Validation Accuracy')
plt.title('Accuracy over Epochs')
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.legend()

# Loss
plt.subplot(1, 3, 2)
plt.plot(history.history['loss'], label='Train Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.title('Loss over Epochs')
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.legend()

# Confusion Matrix
plt.subplot(1, 3, 3)
sns.heatmap(conf_matrix, annot=True, fmt='d', cmap='Blues', xticklabels=label_encoder.classes_, yticklabels=label_encoder.classes_)
plt.title('Confusion Matrix')
plt.xlabel('Predicted')
plt.ylabel('True')

plt.show()

# Classification report
print(classification_report(np.argmax(y_test, axis=1), predicted_labels))

# ROC AUC Score
auc_score = roc_auc_score(y_test, predictions, multi_class='ovr')
print(f"AUC Score: {auc_score:.2f}")

# Save the model
model.save('model.h5')
