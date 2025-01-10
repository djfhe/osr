import pandas as pd
import argparse

def calculate_statistics(file1, file2):
    """
    Reads two CSV files, calculates statistics for the `total(B)` column, and compares the results.
    Args:
        file1 (str): Path to the first CSV file.
        file2 (str): Path to the second CSV file.
    Returns:
        pd.DataFrame: A DataFrame containing statistics and percentage differences.
    """
    # Read the CSV files
    df1 = pd.read_csv(file1)
    df2 = pd.read_csv(file2)

    # Ensure the `total(B)` column exists
    if "total(B)" not in df1.columns or "total(B)" not in df2.columns:
        raise ValueError("Both files must contain a 'total(B)' column.")
    
    # Calculate statistics for `total(B)`
    stats1 = {
        "mean": df1["total(B)"].mean(),
        "median": df1["total(B)"].median(),
        "std_dev": df1["total(B)"].std(),
        "min": df1["total(B)"].min(),
        "max": df1["total(B)"].max(),
    }
    
    stats2 = {
        "mean": df2["total(B)"].mean(),
        "median": df2["total(B)"].median(),
        "std_dev": df2["total(B)"].std(),
        "min": df2["total(B)"].min(),
        "max": df2["total(B)"].max(),
    }

    # Calculate percentage difference
    percentage_diff = {
        key: ((stats2[key] - stats1[key]) / stats1[key]) * 100 for key in stats1.keys()
    }
    
    # Combine results into a DataFrame
    results = pd.DataFrame({
        "Statistic": ["Mean", "Median", "Std Dev", "Min", "Max"],
        "File1": [stats1["mean"], stats1["median"], stats1["std_dev"], stats1["min"], stats1["max"]],
        "File2": [stats2["mean"], stats2["median"], stats2["std_dev"], stats2["min"], stats2["max"]],
        "Percentage Difference (%)": [
            percentage_diff["mean"], 
            percentage_diff["median"], 
            percentage_diff["std_dev"], 
            percentage_diff["min"], 
            percentage_diff["max"]
        ]
    })

    return results

def main():
    # Argument parser for command-line inputs
    parser = argparse.ArgumentParser(description="Calculate statistics and compare the 'total(B)' column between two CSV files.")
    parser.add_argument("input_file1", type=str, help="Path to the first CSV file.")
    parser.add_argument("input_file2", type=str, help="Path to the second CSV file.")
    parser.add_argument("output_file", type=str, help="Path to save the statistics (CSV format).")
    
    # Parse arguments
    args = parser.parse_args()
    
    # Calculate statistics and comparison
    results = calculate_statistics(args.input_file1, args.input_file2)
    
    # Save to CSV
    results.to_csv(args.output_file, index=False)
    print(f"Statistics and comparison saved to {args.output_file}")

if __name__ == "__main__":
    main()
