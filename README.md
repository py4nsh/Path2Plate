# Path2Plate — C++ Food Delivery System

**Team Code_crew | Second-year C++ / Data Structures project**

Path2Plate is a small food-delivery demonstration app. It keeps the **application logic, data processing, file handling and DSA algorithms in C++**. A lightweight HTML/CSS/JavaScript page provides the browser-based graphical interface, and communicates with the C++ program through its built-in local socket server. There is **no Python, Flask, Django, Node.js backend, or external database**.

> If your course rule literally prohibits HTML/CSS/JavaScript as well as Python, confirm that with your instructor: a browser UI needs a presentation layer. In this project, that layer only displays the interface; C++ handles the app operations and stored data.

## Run on Windows

1. Extract this folder to a normal location.
2. Install MSYS2 with the **UCRT64** GCC compiler if needed.
3. Double-click `build_and_run.bat`.
4. When the console reports that the server is running, open <http://localhost:8000> in Chrome or Edge.
5. Keep the console window open while using the app. Press `Ctrl+C` to stop it.

The batch file checks the common MSYS2 UCRT64 locations, including `%USERPROFILE%\Downloads\ucrt64\bin` and `C:\msys64\ucrt64\bin`, and compiles with C++14 plus the Windows socket library.

## Run on Linux / macOS

```bash
g++ -std=c++14 Path2Plate.cpp -o Path2Plate
./Path2Plate
```

Then open <http://localhost:8000>.

Run the program from this folder because it loads `index.html` and creates its `data/` folder relative to the current working directory.

## Features

- Customer registration and login using a 10-digit phone number
- Restaurant search and menu browsing
- Cart quantity controls and bill calculation (subtotal, delivery fee and GST)
- Payment choice after selecting food: Cash on Delivery, UPI Demo and Card Demo
- Order confirmation, order history and delivery-status tracking
- Admin page to view orders, process the FIFO queue, update delivery status and manage menu items
- Shortest delivery route using a weighted graph and Dijkstra's algorithm

### Payment demonstration only

UPI Demo and Card Demo simulate a successful payment for a classroom presentation. No payment gateway is connected, no money is transferred, and the app does not request real PINs, card numbers or CVVs.

## C++ / DSA concepts

- Structures, functions, vectors and string handling
- File handling with `ifstream` and `ofstream`
- `queue` for pending orders (FIFO)
- Weighted graph represented with an adjacency list
- `priority_queue` and Dijkstra's shortest-path algorithm
- Basic client/server socket communication in C++

## File-based storage

The app creates `data/` on first run and stores records in text files: `locations.txt`, `roads.txt`, `restaurants.txt`, `menu.txt`, `customers.txt` and `orders.txt`. This is a simple file-based store for a student project, not a full DBMS.

## Admin

Default demonstration PIN: `1234`.

This is a classroom project, not a production food-delivery or payment system.

## UI refresh (November 2026)

The browser interface has a refreshed warm cream, orange and deep-green colour palette, responsive navigation, a welcome banner, cuisine filters, restaurant search and sorting, locally saved favourite restaurants, clearer menu item rows, and a more structured checkout summary. These are frontend improvements; the original C++ API, text-file storage, queue, graph and Dijkstra route logic remain in place.

Favourites are stored in the browser's local storage on the current device. They are not synced between devices or stored in the C++ data files. Restaurant ratings, real-time maps, live payment processing, coupon redemption and OTP authentication are not implemented in this student demo. UPI and card options remain simulated, and the default admin PIN remains `1234` for classroom demonstration only. Do not deploy this project as a real payment or customer-data service without adding proper authentication, authorization, input/output safety and persistent database controls.


## Recent UI improvements
- Delivery tracking now includes a simple animated scooter marker on the Dijkstra route and a clearer order-status timeline.
- Admin view includes summary cards, order-status progress bars, delivered revenue, customer/restaurant counts and average route distance.
- The rider animation is a visual demo only; it does not use GPS or represent a real delivery partner location.
- Order confirmation has a reference-inspired layout with a four-step progress tracker, route map, animated scooter illustration, estimated delivery time, item summary and payment total, styled in Path2Plate's cream, forest-green and orange theme.
