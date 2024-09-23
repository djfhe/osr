import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Load CSV file
file_path = 'results.csv'  # Replace with your actual file path
data = pd.read_csv(file_path)

# Calculate statistical data
stats = data.groupby('thread index')['duration in microseconds'].agg(
    average='mean',
    median='median',
    min='min',
    max='max',
    percentile_25=lambda x: np.percentile(x, 25),
    percentile_50=lambda x: np.percentile(x, 50),
    percentile_75=lambda x: np.percentile(x, 75),
).reset_index()

# Display the statistical data
print("Statistical Summary:")
print(stats)

# Plot histogram
plt.figure(figsize=(10, 6))
for thread_index in data['thread index'].unique():
    plt.hist(data[data['thread index'] == thread_index]['duration in microseconds'], 
             bins=30, alpha=0.5, label=f'Thread {thread_index}')

plt.xlabel('Duration in microseconds')
plt.ylabel('Frequency')
plt.title('Histogram of Program Durations by Thread')
plt.legend(loc='upper right')
plt.show()

# Save statistical data to a CSV file
output_path = 'thread_stats.csv'
stats.to_csv(output_path, index=False)
print(f"\nStatistical data saved to {output_path}")