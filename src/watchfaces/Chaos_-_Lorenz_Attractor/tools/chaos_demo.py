#!/usr/bin/env python3
"""
Lorenz Attractor Watchy E-ink GIF Generator (Simple Version)

Copyright (c) 2024 Ajey Pai Karkala

This script generates a GIF animation showing how the Lorenz attractor
would appear on the Watchy e-ink display, simulating the actual watch face.
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.animation import FuncAnimation
import math
from PIL import Image, ImageDraw, ImageFont
import io
from datetime import datetime

class LorenzSimulator:
    def __init__(self):
        # Lorenz system parameters (matching Arduino code)
        self.sigma = 10.0
        self.rho = 28.0
        self.beta = 8.0/3.0
        self.dt = 0.05
        
        # Initial position
        self.pos = np.array([1.0, 1.0, 1.0])
        
        # Trajectory storage
        self.max_points = 500
        self.trajectory = []
        self.rotation_angle = 0.0
        
        # Watchy display parameters
        self.display_width = 200
        self.display_height = 200
        
        # Configuration (matching settings.h)
        self.timezone_offset_hours = 1  # Amsterdam (CET)
        self.latitude = 52.3676  # Amsterdam
        self.longitude = 4.9041  # Amsterdam
        
    def lorenz_derivative(self, p):
        """Calculate Lorenz system derivatives"""
        x, y, z = p
        dx = self.sigma * (y - x)
        dy = x * (self.rho - z) - y
        dz = x * y - self.beta * z
        return np.array([dx, dy, dz])
    
    def update_lorenz(self):
        """Runge-Kutta 4th order integration"""
        k1 = self.lorenz_derivative(self.pos)
        k2 = self.lorenz_derivative(self.pos + k1 * self.dt / 2)
        k3 = self.lorenz_derivative(self.pos + k2 * self.dt / 2)
        k4 = self.lorenz_derivative(self.pos + k3 * self.dt)
        
        self.pos += (k1 + 2*k2 + 2*k3 + k4) * self.dt / 6
    
    def project_3d_to_2d(self, point_3d):
        """Project 3D point to 2D with rotation"""
        x, y, z = point_3d
        
        # Apply Y-axis rotation
        cos_rot = math.cos(self.rotation_angle)
        sin_rot = math.sin(self.rotation_angle)
        
        x_rot = x * cos_rot - z * sin_rot
        y_rot = y
        
        return x_rot, y_rot
    
    def add_trajectory_point(self):
        """Add current position to trajectory"""
        if len(self.trajectory) < self.max_points:
            self.trajectory.append(self.pos.copy())
        else:
            # Circular buffer
            self.trajectory.pop(0)
            self.trajectory.append(self.pos.copy())
    
    def get_screen_coordinates(self):
        """Convert trajectory to screen coordinates"""
        if not self.trajectory:
            return [], []
        
        # Project all points to 2D
        screen_points = []
        for point in self.trajectory:
            x_2d, y_2d = self.project_3d_to_2d(point)
            screen_points.append([x_2d, y_2d])
        
        screen_points = np.array(screen_points)
        
        if len(screen_points) == 0:
            return [], []
        
        # Find bounds
        min_x, max_x = screen_points[:, 0].min(), screen_points[:, 0].max()
        min_y, max_y = screen_points[:, 1].min(), screen_points[:, 1].max()
        
        # Calculate scale factor
        range_x = max_x - min_x
        range_y = max_y - min_y
        
        if range_x < 0.1:
            range_x = 0.1
        if range_y < 0.1:
            range_y = 0.1
        
        scale_x = 80.0 / range_x
        scale_y = 80.0 / range_y
        scale = min(scale_x, scale_y)
        
        # Center and scale
        center_x = (min_x + max_x) / 2
        center_y = (min_y + max_y) / 2
        
        screen_x = (screen_points[:, 0] - center_x) * scale + 100
        screen_y = (screen_points[:, 1] - center_y) * scale + 100
        
        # Clamp to screen bounds
        screen_x = np.clip(screen_x, 5, 195)
        screen_y = np.clip(screen_y, 5, 195)
        
        return screen_x, screen_y
    
    def simulate_frame(self):
        """Simulate one frame of the animation"""
        # Add 5 points per frame (matching Arduino code)
        for _ in range(5):
            self.update_lorenz()
            self.add_trajectory_point()
        
        # Rotate view (matching LORENZ_ROTATION_SPEED = 0.0523598776)
        self.rotation_angle += 0.0523598776
        if self.rotation_angle > 2 * math.pi:
            self.rotation_angle -= 2 * math.pi

def calculate_moon_phase(year, month, day, hour, minute, timezone_offset):
    """Calculate moon phase (0.0 = new moon, 0.5 = full moon)"""
    MOON_CYCLE_DAYS = 29.53058867
    
    # Calculate days since 2000-01-01
    days_since_2000 = 0
    for y in range(2000, year):
        days_since_2000 += 366 if (y % 4 == 0 and (y % 100 != 0 or y % 400 == 0)) else 365
    
    days_in_month = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    if year % 4 == 0 and (year % 100 != 0 or year % 400 == 0):
        days_in_month[1] = 29
    
    for m in range(1, month):
        days_since_2000 += days_in_month[m - 1]
    
    days_since_2000 += day - 1
    
    utc_seconds = days_since_2000 * 86400
    utc_seconds += hour * 3600 + minute * 60
    utc_seconds -= timezone_offset * 3600
    
    REF_NEW_MOON_SECONDS = (5.0 + 18.0/24.0 + 14.0/1440.0) * 86400.0
    
    seconds_since_new_moon = utc_seconds - REF_NEW_MOON_SECONDS
    days_since_new_moon = seconds_since_new_moon / 86400.0
    
    days_in_cycle = days_since_new_moon % MOON_CYCLE_DAYS
    if days_in_cycle < 0:
        days_in_cycle += MOON_CYCLE_DAYS
    
    return days_in_cycle / MOON_CYCLE_DAYS

def calculate_sunrise_sunset(latitude, longitude, timezone_offset, year, month, day):
    """Calculate sunrise and sunset times"""
    import math
    
    lat_rad = math.radians(latitude)
    
    # Calculate day of year
    days_in_month = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    if year % 4 == 0 and (year % 100 != 0 or year % 400 == 0):
        days_in_month[1] = 29
    
    day_of_year = day
    for m in range(1, month):
        day_of_year += days_in_month[m - 1]
    
    # Solar declination
    declination = math.radians(23.45) * math.sin(2.0 * math.pi * (284.0 + day_of_year) / 365.0)
    
    # Solar elevation angle for sunrise/sunset
    solar_elevation = math.radians(-0.83)
    
    # Hour angle
    cos_hour_angle = (math.sin(solar_elevation) - math.sin(lat_rad) * math.sin(declination)) / (math.cos(lat_rad) * math.cos(declination))
    
    if cos_hour_angle > 1.0:
        return -1.0, -1.0  # No sunrise/sunset (polar day/night)
    if cos_hour_angle < -1.0:
        return 0.0, 24.0  # 24-hour day
    
    hour_angle = math.acos(cos_hour_angle)
    
    # Equation of time
    B = 2.0 * math.pi * (day_of_year - 81) / 365.0
    equation_of_time = 9.87 * math.sin(2.0 * B) - 7.53 * math.cos(B) - 1.5 * math.sin(B)
    
    # Time correction
    longitude_time_offset = longitude / 15.0
    time_correction = longitude_time_offset - timezone_offset + (equation_of_time / 60.0)
    
    solar_noon = 12.0 - time_correction
    
    sunrise_solar = solar_noon - (hour_angle * 12.0 / math.pi)
    sunset_solar = solar_noon + (hour_angle * 12.0 / math.pi)
    
    sunrise_hour = sunrise_solar
    sunset_hour = sunset_solar
    
    if sunrise_hour < 0:
        sunrise_hour += 24.0
    if sunrise_hour >= 24.0:
        sunrise_hour -= 24.0
    if sunset_hour < 0:
        sunset_hour += 24.0
    if sunset_hour >= 24.0:
        sunset_hour -= 24.0
    
    return sunrise_hour, sunset_hour

def draw_moon_phase(draw, phase, moon_x, moon_y, moon_radius):
    """Draw moon phase indicator"""
    waxing = phase < 0.5
    illumination = phase * 2.0 if phase < 0.5 else (1.0 - phase) * 2.0
    
    for x in range(-moon_radius, moon_radius + 1):
        for y in range(-moon_radius, moon_radius + 1):
            dist = math.sqrt(x*x + y*y)
            px = moon_x + x
            py = moon_y + y
            
            if px < 0 or px >= 200 or py < 0 or py >= 200:
                continue
            
            # Draw circle outline
            if dist > moon_radius - 0.5 and dist <= moon_radius + 0.5:
                draw.point((px, py), fill=0)
            # Fill based on phase
            elif dist < moon_radius - 1:
                should_fill = False
                
                if illumination > 0.95:
                    should_fill = True
                elif illumination < 0.05:
                    should_fill = False
                else:
                    x_pos = x / float(moon_radius)
                    if waxing:
                        threshold = -1.0 + 2.0 * illumination
                        should_fill = (x_pos >= threshold)
                    else:
                        threshold = 1.0 - 2.0 * illumination
                        should_fill = (x_pos <= threshold)
                
                if should_fill:
                    draw.point((px, py), fill=0)

def create_watchy_frame_pil(simulator, frame_num):
    """Create a single frame using PIL for better e-ink simulation"""
    # Simulate the frame
    simulator.simulate_frame()
    
    # Get screen coordinates
    screen_x, screen_y = simulator.get_screen_coordinates()
    
    # Create image (200x200, white background)
    img = Image.new('L', (200, 200), 255)  # 'L' = grayscale, 255 = white
    draw = ImageDraw.Draw(img)
    
    # Draw trajectory points (2x2 squares)
    for x, y in zip(screen_x, screen_y):
        # Draw 2x2 square for each point
        for dx in range(-1, 1):
            for dy in range(-1, 1):
                if 0 <= x + dx < 200 and 0 <= y + dy < 200:
                    draw.point((int(x + dx), int(y + dy)), fill=0)  # 0 = black
    
    # Draw current position (3x3 square) - last point
    if len(screen_x) > 0:
        current_x = int(screen_x[-1])
        current_y = int(screen_y[-1])
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                if 0 <= current_x + dx < 200 and 0 <= current_y + dy < 200:
                    draw.point((current_x + dx, current_y + dy), fill=0)
    
    # Draw watch elements
    try:
        # Try to use a monospace font
        font = ImageFont.truetype("/System/Library/Fonts/Monaco.ttf", 12)
    except:
        try:
            font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 12)
        except:
            font = ImageFont.load_default()
    
    # Time (fixed)
    time_str = "14:23"
    draw.text((5, 5), time_str, fill=0, font=font)
    
    # Get time width for date positioning
    bbox = draw.textbbox((0, 0), time_str, font=font)
    time_width = bbox[2] - bbox[0]
    time_end_x = 5 + time_width
    
    # Moon phase
    moon_radius = 10
    moon_x = 200 - moon_radius - 8
    moon_y = moon_radius + 2
    
    # Calculate moon phase for demo date (2024-03-15 14:23)
    moon_phase = calculate_moon_phase(2024, 3, 15, 14, 23, simulator.timezone_offset_hours)
    draw_moon_phase(draw, moon_phase, moon_x, moon_y, moon_radius)
    
    # Date (fixed) - positioned to avoid moon overlap
    # Calculate day of week for demo date (2024-03-15)
    # Python weekday(): Monday=0, Tuesday=1, ..., Sunday=6
    demo_date = datetime(2024, 3, 15)
    day_names = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
    day_name = day_names[demo_date.weekday()]
    date_str = f"{day_name} 15/03"
    bbox = draw.textbbox((0, 0), date_str, font=font)
    date_width = bbox[2] - bbox[0]
    moon_left_edge = moon_x - moon_radius
    max_date_x = moon_left_edge - 5
    
    date_start_x = time_end_x + 12
    if date_start_x + date_width > max_date_x:
        date_start_x = max_date_x - date_width
        if date_start_x < time_end_x + 5:
            date_start_x = time_end_x + 5
    
    if date_start_x + date_width > 200:
        date_start_x = 200 - date_width
    
    draw.text((date_start_x, 5), date_str, fill=0, font=font)
    
    # Battery icon (percentage-based, showing ~75% for demo)
    battery_percent = 0.75
    battery_x = 200 - 25
    battery_y = 200 - 15
    battery_width = 18
    battery_height = 10
    
    # Battery outline
    draw.rectangle([battery_x, battery_y, battery_x + battery_width, battery_y + battery_height], outline=0, width=1)
    # Battery terminal
    draw.rectangle([battery_x + battery_width, battery_y + 2, battery_x + battery_width + 2, battery_y + 8], fill=0)
    # Fill battery based on percentage
    fill_width = int((battery_width - 2) * battery_percent)
    if fill_width > 0:
        draw.rectangle([battery_x + 1, battery_y + 1, battery_x + 1 + fill_width, battery_y + battery_height - 1], fill=0)
    
    # Steps (fixed)
    steps = "2847"
    draw.text((5, 185), steps, fill=0, font=font)
    
    # Sunrise/Sunset (centered at bottom)
    sunrise_hour, sunset_hour = calculate_sunrise_sunset(
        simulator.latitude, simulator.longitude, simulator.timezone_offset_hours, 2024, 3, 15
    )
    
    if sunrise_hour >= 0 and sunset_hour >= 0:
        sunrise_h = int(sunrise_hour)
        sunrise_m = int((sunrise_hour - sunrise_h) * 60.0)
        sunset_h = int(sunset_hour)
        sunset_m = int((sunset_hour - sunset_h) * 60.0)
        
        sunrise_str = f"{sunrise_h:02d}{sunrise_m:02d}"
        sunset_str = f"{sunset_h:02d}{sunset_m:02d}"
        display_str = f"{sunrise_str}/{sunset_str}"
        
        bbox = draw.textbbox((0, 0), display_str, font=font)
        text_width = bbox[2] - bbox[0]
        center_x = (200 - text_width) // 2
        draw.text((center_x, 185), display_str, fill=0, font=font)
    
    return img

def generate_gif():
    """Generate the complete GIF animation"""
    print("Generating Lorenz Attractor Watchy GIF...")
    
    # Create simulator
    simulator = LorenzSimulator()
    
    # Generate frames
    num_frames = 60  # More frames for smoother animation
    frames = []
    
    for frame in range(num_frames):
        print(f"Generating frame {frame + 1}/{num_frames}")
        img = create_watchy_frame_pil(simulator, frame)
        frames.append(img)
    
    # Save as GIF
    output_file = "../examples/gifs/chaos_watchy_demo.gif"
    frames[0].save(
        output_file,
        save_all=True,
        append_images=frames[1:],
        duration=100,  # 100ms per frame (10 FPS)
        loop=0
    )
    
    print(f"GIF saved as: {output_file}")
    print(f"Animation shows {num_frames} frames at 10 FPS")
    print("Features included:")
    print("  - Lorenz attractor with 3D rotation")
    print("  - Moon phase indicator")
    print("  - Sunrise/sunset times")
    print("  - Percentage-based battery indicator")
    print("  - Time, date, and step counter")

if __name__ == "__main__":
    generate_gif()
