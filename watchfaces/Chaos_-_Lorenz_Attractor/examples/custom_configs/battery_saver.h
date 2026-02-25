/*
 * Battery Saver Configuration
 * Optimized for maximum battery life with minimal pattern evolution
 * Expected battery life: 10-14 days
 * 
 * Copyright (c) 2024 Ajey Pai Karkala
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BATTERY_SAVER_H
#define BATTERY_SAVER_H

// Wake up settings - slower updates for battery saving
#define WAKE_UP_INTERVAL_MINUTES 1.0  // 1 minute

// Lorenz attractor settings - minimal processing
#define LORENZ_POINTS_PER_UPDATE 20   // Fewer points per update
#define LORENZ_MAX_POINTS 200         // Shorter trails
#define LORENZ_ROTATION_SPEED 0.2     // Slow rotation

// Display refresh settings - infrequent full refreshes to save battery
#define FULL_REFRESH_INTERVAL 120     // Full refresh every 2 hours (120 × 1min)

// Location and timezone settings (update for your location)
#define TIMEZONE_OFFSET_HOURS 1       // Amsterdam (CET) - change to 2 for CEST (summer time)
#define LATITUDE 52.3676              // Latitude in degrees
#define LONGITUDE 4.9041              // Longitude in degrees

#endif
