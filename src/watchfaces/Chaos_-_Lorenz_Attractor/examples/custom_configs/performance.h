/*
 * Performance Configuration
 * Optimized for maximum visual impact and fast evolution
 * Expected battery life: 3-5 days
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

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

// Wake up settings - fast updates for maximum visual impact
#define WAKE_UP_INTERVAL_MINUTES 0.05  // 3 seconds

// Lorenz attractor settings - maximum processing
#define LORENZ_POINTS_PER_UPDATE 100   // Many points per update
#define LORENZ_MAX_POINTS 500          // Long trails
#define LORENZ_ROTATION_SPEED 1.0      // Fast rotation

// Display refresh settings - frequent full refreshes for clean display
#define FULL_REFRESH_INTERVAL 10       // Full refresh every 30 seconds (10 × 3s)

// Location and timezone settings (update for your location)
#define TIMEZONE_OFFSET_HOURS 1        // Amsterdam (CET) - change to 2 for CEST (summer time)
#define LATITUDE 52.3676               // Latitude in degrees
#define LONGITUDE 4.9041               // Longitude in degrees

#endif
