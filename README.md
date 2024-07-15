# Parking lot monitor demo
A very rough demo to try to detect car parking in a parking lot.
## Constraint
- A car can occlude next parking lot space.
- Car occluded each other.
## Approach
- Label entire area of parking lot, but doesn't label each lot separately.
- Detect a car and track it.
- If a car is detect within parking lot area, record the first time we seen it and last time we seen it.
- Keep updating last seen time, report elapse time between first seen and last seen as parking time.

[Demo video](https://youtu.be/XUCbxhc3ydg)
