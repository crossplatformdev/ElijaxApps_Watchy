/*
 * Balanced Configuration (Default)
 * Good balance of visual impact and battery life
 * Expected battery life: 7-10 days
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

#ifndef BALANCED_H
#define BALANCED_H

// Wake up settings - balanced updates
#define WAKE_UP_INTERVAL_MINUTES 0.05  // 3 seconds

// Lorenz attractor settings - balanced processing
#define LORENZ_POINTS_PER_UPDATE 50    // Moderate points per update
#define LORENZ_MAX_POINTS 300          // Medium trails
#define LORENZ_ROTATION_SPEED 0.5      // Moderate rotation

// Display refresh settings - balanced refresh strategy
#define FULL_REFRESH_INTERVAL 20       // Full refresh every 60 seconds (20 × 3s)

// Location and timezone settings (update for your location)
#define TIMEZONE_OFFSET_HOURS 1        // Amsterdam (CET) - change to 2 for CEST (summer time)
#define LATITUDE 52.3676               // Latitude in degrees
#define LONGITUDE 4.9041               // Longitude in degrees

#endif
