import os
import glob
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

def gaussian_plus_offset(x, amplitude, mean, sigma, offset):
    return amplitude * np.exp(-((x - mean) ** 2) / (2 * sigma ** 2)) + offset

def analyze_laser_profiles(data_dir):
    # Constants from C++ code
    PIXEL_SIZE = 0.00345  # mm
    FIT_RANGE_MM = 3.0    # +/- 3.0 mm for fitting

    # Find all DeviceData CSV files
    file_pattern = os.path.join(data_dir, "DeviceData_#*.csv")
    files = sorted(glob.glob(file_pattern))
    
    if not files:
        print(f"No files found in {data_dir}")
        return

    results = []
    global_max_val = -1
    brightest_file_path = None
    brightest_data = None
    brightest_params = None

    summary_lines = []
    print(f"{'File':<10} | {'Max Int':<10} | {'Center X':<10} | {'FWHM X':<10} | {'Center Y':<10} | {'FWHM Y':<10}")
    print("-" * 75)

    for file_path in files:
        file_name = os.path.basename(file_path)
        try:
            # Read data, skipping first 18 lines
            # Using pandas for speed, assuming semi-colon delimiter
            # header=None because we skip header lines manually
            # We need to handle potentially varying line lengths or trailing semicolons
            
            # Reading with numpy genfromtxt might be safer for irregular lines if pandas fails, 
            # but pandas read_csv with sep=';' is usually good.
            # The C++ code reads line by line. Let's try pandas first.
            
            # Note: The file has a trailing semicolon in the C++ code logic? 
            # "while (std::getline(iss, token, ';')" 
            # Let's check the line content I read earlier: "72;62;...;102" -> looks like no trailing semi-colon usually, 
            # but let's be careful.
            
            # Pandas might interpret the last empty column if line ends with ;
            df = pd.read_csv(file_path, skiprows=18, sep=';', header=None, engine='python', encoding='shift_jis')
            
            # Drop any all-NaN columns (sometimes trailing ;)
            df = df.dropna(axis=1, how='all')
            
            data = df.values
            height, width = data.shape
            
            # Find max location
            max_val = np.nanmax(data)
            max_indices = np.unravel_index(np.nanargmax(data), data.shape)
            peak_y_idx, peak_x_idx = max_indices # numpy is (row, col) -> (y, x)
            
            # Convert to mm
            peak_x_mm = peak_x_idx * PIXEL_SIZE
            peak_y_mm = (height - 1 - peak_y_idx) * PIXEL_SIZE # Y is inverted in C++ code: (height - 1 - y_idx)
            
            # Extract profiles
            # X profile: row at peak_y_idx
            profile_x = data[peak_y_idx, :]
            x_axis = np.arange(width) * PIXEL_SIZE
            
            # Y profile: col at peak_x_idx
            profile_y = data[:, peak_x_idx]
            # Y axis needs to be consistent with C++ code: 0 at bottom?
            # In C++: h2->Fill(x_idx * pixel_size, (height - 1 - y_idx) * pixel_size, val);
            # So row 0 (top of file) corresponds to Y = (height-1)*pixel_size (max Y)
            # And row height-1 (bottom of file) corresponds to Y = 0.
            # So profile_y array index 0 is High Y, index max is Low Y.
            # We want to plot against Y coordinate.
            y_axis = (height - 1 - np.arange(height)) * PIXEL_SIZE
            
            # Sort Y for plotting/fitting because y_axis is descending
            y_sort_idx = np.argsort(y_axis)
            y_axis_sorted = y_axis[y_sort_idx]
            profile_y_sorted = profile_y[y_sort_idx]

            # Define fitting function
            def fit_profile(axis, profile, center_guess):
                # Select range
                mask = (axis >= center_guess - FIT_RANGE_MM) & (axis <= center_guess + FIT_RANGE_MM)
                x_fit = axis[mask]
                y_fit = profile[mask]
                
                if len(x_fit) < 5:
                    return None, None
                
                # Initial guesses: amplitude, mean, sigma, offset
                p0 = [max_val, center_guess, 0.5, 200]
                
                try:
                    popt, pcov = curve_fit(gaussian_plus_offset, x_fit, y_fit, p0=p0, maxfev=2000)
                    return popt, x_fit
                except:
                    return None, None

            # Fit X
            popt_x, x_fit_data = fit_profile(x_axis, profile_x, peak_x_mm)
            if popt_x is not None:
                fit_center_x = popt_x[1]
                fit_fwhm_x = 2.355 * abs(popt_x[2])
            else:
                fit_center_x = peak_x_mm
                fit_fwhm_x = 0
            
            # Fit Y
            popt_y, y_fit_data = fit_profile(y_axis_sorted, profile_y_sorted, peak_y_mm)
            if popt_y is not None:
                fit_center_y = popt_y[1]
                fit_fwhm_y = 2.355 * abs(popt_y[2])
            else:
                fit_center_y = peak_y_mm
                fit_fwhm_y = 0
            
            file_num_str = file_name.split('#')[1].split('.')[0]
            results.append({
                'File': file_num_str,
                'Max_Intensity': max_val,
                'Center_X[mm]': fit_center_x,
                'FWHM_X[mm]': fit_fwhm_x,
                'Center_Y[mm]': fit_center_y,
                'FWHM_Y[mm]': fit_fwhm_y
            })
            
            print(f"{file_num_str:<10} | {max_val:<10.1f} | {fit_center_x:<10.3f} | {fit_fwhm_x:<10.3f} | {fit_center_y:<10.3f} | {fit_fwhm_y:<10.3f}")

            if max_val > global_max_val:
                global_max_val = max_val
                brightest_file_path = file_path
                brightest_data = data
                brightest_params = {
                    'x_axis': x_axis,
                    'y_axis': y_axis, # Original unsorted Y axis (descending)
                    'profile_x': profile_x,
                    'profile_y': profile_y,
                    'popt_x': popt_x,
                    'popt_y': popt_y,
                    'peak_x_mm': peak_x_mm,
                    'peak_y_mm': peak_y_mm,
                    'fwhm_x': fit_fwhm_x,
                    'fwhm_y': fit_fwhm_y,
                    'fit_center_x': fit_center_x,
                    'fit_center_y': fit_center_y
                }

        except Exception as e:
            print(f"Error processing {file_name}: {e}")

    # Save results
    df_results = pd.DataFrame(results)
    df_results.to_csv(os.path.join(data_dir, "laser_analysis_summary.csv"), index=False)
    print(f"\nResults saved to {os.path.join(data_dir, 'laser_analysis_summary.csv')}")

    # Plot brightest
    if brightest_params:
        print(f"\nPlotting brightest profile: {os.path.basename(brightest_file_path)}")
        
        # 1. Fit Plot
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
        
        # X Fit
        x_ax = brightest_params['x_axis']
        prof_x = brightest_params['profile_x']
        ax1.plot(x_ax, prof_x, label='Data')
        if brightest_params['popt_x'] is not None:
            fit_x = gaussian_plus_offset(x_ax, *brightest_params['popt_x'])
            # Only plot fit in range roughly
            ax1.plot(x_ax, fit_x, 'r-', label=f"Fit\nCenter: {brightest_params['fit_center_x']:.3f}\nFWHM: {brightest_params['fwhm_x']:.3f}")
        ax1.set_title("X Profile")
        ax1.set_xlabel("X [mm]")
        ax1.set_ylabel("Intensity")
        ax1.legend()
        ax1.set_xlim(brightest_params['fit_center_x'] - 3, brightest_params['fit_center_x'] + 3)

        # Y Fit
        # We need to sort Y for plotting lines correctly if we just used plot
        y_ax = brightest_params['y_axis']
        prof_y = brightest_params['profile_y']
        # Sort for plotting
        idx = np.argsort(y_ax)
        y_ax_s = y_ax[idx]
        prof_y_s = prof_y[idx]
        
        ax2.plot(y_ax_s, prof_y_s, label='Data')
        if brightest_params['popt_y'] is not None:
            fit_y = gaussian_plus_offset(y_ax_s, *brightest_params['popt_y'])
            ax2.plot(y_ax_s, fit_y, 'r-', label=f"Fit\nCenter: {brightest_params['fit_center_y']:.3f}\nFWHM: {brightest_params['fwhm_y']:.3f}")
        ax2.set_title("Y Profile")
        ax2.set_xlabel("Y [mm]")
        ax2.set_ylabel("Intensity")
        ax2.legend()
        ax2.set_xlim(brightest_params['fit_center_y'] - 3, brightest_params['fit_center_y'] + 3)
        
        plt.tight_layout()
        plt.savefig(os.path.join(data_dir, "laser_brightest_profile_fit.png"))
        plt.close()
        
        # 2. Map Plot
        plt.figure(figsize=(10, 8))
        # Extent: [left, right, bottom, top]
        # X goes 0 to max. Y goes 0 to max. 
        # Data is stored [row, col]. row 0 is Y_max. row N is Y_0.
        # imshow displays [0,0] at top-left by default.
        # So we need extent=[0, x_max, y_min, y_max] and origin='upper' should match the C++ logic?
        # C++: h2->Fill(x, y, val).
        # If we use origin='lower', row 0 is bottom. But our row 0 is high Y.
        # So we use origin='upper' (default) and correct extent?
        # No, easier to stick to coordinates.
        # In C++ Fill: x is col index, y is (height-1-row)*pixel_size.
        # So row 0 corresponds to y_max. Row height-1 corresponds to y=0.
        # If we plot with imshow(data), row 0 is top.
        # If we set extent=[0, width*PS, 0, height*PS], we need origin='lower' IF row 0 was y=0.
        # But row 0 is y=max.
        # So standard imshow (origin='upper') puts row 0 at top.
        # We want the Y axis to show mm from 0 (bottom) to max (top).
        # So we should flip the data vertically or use origin='lower' and appropriate extent.
        # Let's use origin='lower' and flip data up-down so row 0 (originally top/high Y) becomes top visually.
        # Wait, if row 0 is high Y, and we want Y axis 0 to max upwards:
        # row 0 (data[0]) is at Y=max. row N is at Y=0.
        # If we display this as an image with Y=0 at bottom:
        # We should put data[N] at bottom.
        # So we flipud(data) to make data[N] the first row for origin='lower'?
        # np.flipud(data) makes row N (the last row) the first row (index 0).
        # If we use origin='lower', index 0 is drawn at bottom.
        # So yes, flipud + origin='lower' + extent=[0, x_max, 0, y_max] works.
        
        h, w = brightest_data.shape
        x_max = w * PIXEL_SIZE
        y_max = h * PIXEL_SIZE
        
        plt.imshow(np.flipud(brightest_data), extent=[0, x_max, 0, y_max], origin='lower', cmap='jet', aspect='equal')
        plt.colorbar(label='Intensity')
        
        # Add markers
        plt.plot(brightest_params['fit_center_x'], brightest_params['fit_center_y'], 'w+', markersize=15, markeredgewidth=2)
        
        # Circle (Ellipse)
        ellipse = plt.Circle((brightest_params['fit_center_x'], brightest_params['fit_center_y']), 
                             radius=max(brightest_params['fwhm_x'], brightest_params['fwhm_y'])/2,
                             color='w', fill=False, linestyle='--', linewidth=2)
        plt.gca().add_patch(ellipse)
        
        plt.title(f"Brightest Profile (File #{os.path.basename(brightest_file_path).split('#')[1].split('.')[0]})")
        plt.xlabel("X [mm]")
        plt.ylabel("Y [mm]")
        plt.savefig(os.path.join(data_dir, "laser_brightest_profile_map.png"))
        plt.close()

if __name__ == "__main__":
    target_dir = r"20251117-18/20251117/profile/20251117/"
    # Adjust path relative to CWD if needed
    if not os.path.exists(target_dir):
        # Try absolute path or relative from workspace root
        # workspace root: C:\Users\ichin\Documents\research\Ps-\MeasureDensity
        # provided path seems relative.
        pass
        
    analyze_laser_profiles(target_dir)

