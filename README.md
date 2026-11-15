# 🍽️ Path2Plate – C++ Food Ordering & Delivery System

**Team Code_crew | Second-Year C++ & Data Structures and Algorithms Project**

Path2Plate is a food ordering and delivery application developed using C++. It allows users to browse restaurants, explore food menus, add items to their cart, calculate bills, select payment methods, place orders and track delivery status.

The project demonstrates the practical use of Data Structures and Algorithms through features such as FIFO order management and shortest delivery route calculation using Dijkstra's algorithm.

The application uses C++ for its backend operations, data processing, file handling and DSA implementation. A browser-based interface built using HTML, CSS and JavaScript communicates with the C++ backend through a local socket server.

---

## 🚀 Features

### Customer Features

- Customer registration and login using a 10-digit mobile number.
- Restaurant browsing and searching.
- Food menu display with prices and vegetarian/non-vegetarian options.
- Add food items to cart.
- Increase or decrease item quantities.
- Automatic bill calculation.
- GST and delivery charge calculation.
- Multiple payment options.
- Order confirmation and order history.
- Delivery status tracking.
- Shortest delivery route calculation.

### Payment Features

Users can select from the following payment methods:

- Cash on Delivery (COD)
- UPI Demo
- Card Demo

**Note:** UPI and Card payments are simulated for demonstration purposes. No real payment gateway is integrated, and no actual money is transferred.

### Admin Features

- Admin access using a demonstration PIN.
- View customer orders.
- View order details and payment information.
- Process pending orders using a FIFO queue.
- Update order delivery status.
- Manage restaurant menu items.
- View order and delivery-related statistics.

### Delivery Features

- Display delivery route between restaurant and customer locations.
- Calculate the shortest route using Dijkstra's algorithm.
- Display delivery distance and estimated delivery time.
- Track order progress through different delivery stages.
- Visual delivery tracking with an animated scooter.

**Note:** The scooter animation is for visual demonstration only. It does not use real GPS or live delivery partner tracking.

---

## 🛠️ Technology Stack

| **Technology**     | **Purpose**                                    |
| ------------------ | ---------------------------------------------- |
| C++14              | Backend logic and application functionality    |
| HTML5              | Frontend structure                             |
| CSS3               | Styling and responsive UI                      |
| JavaScript         | Frontend interactions                          |
| C++ STL            | Data structures and algorithms                 |
| File Handling      | Persistent storage using text files            |
| Socket Programming | Communication between frontend and C++ backend |
| JSON               | Data exchange between frontend and backend     |
| GCC / MinGW        | C++ compilation                                |

**Important:** The main application logic, order processing, billing, file handling and DSA algorithms are implemented in C++. HTML, CSS and JavaScript are used only for the browser-based user interface.

No Python, Flask, Django, Node.js backend or external database is used.

---

## 🧠 Data Structures & Algorithms Used

The project implements the following C++ and DSA concepts:

### 1. Structures (`struct`)

Structures are used to group related information into a single data type.

Examples:

- Location
- Road
- Restaurant
- FoodItem
- Customer
- Order
- OrderLine
- Bill
- RouteResult

### 2. Classes and Objects

Classes are used to group related data and functions into reusable units. Objects are created from these classes to work with the application's data.

This helps organize customer, restaurant, food, order and delivery-related operations.

### 3. Encapsulation

Encapsulation combines data members and the functions that operate on them inside a class. Access specifiers such as `private` and `public` help control access to class data.

### 4. Vectors

Vectors are used to store collections of data dynamically.

Examples:

- Customer records
- Restaurant details
- Food menu items
- Orders
- Locations and roads

### 5. Queue (FIFO)

A queue is used to manage pending delivery orders.

**FIFO:** First In, First Out.

The order placed first is processed first.

STL operations used:

- `push()`
- `front()`
- `pop()`
- `empty()`

### 6. Graph

A weighted graph represents different delivery locations and roads connecting them.

- Vertices represent locations.
- Edges represent roads.
- Weights represent road distances in kilometres.

### 7. Adjacency List

The graph is represented using an adjacency list.

Each location stores its neighbouring locations along with the corresponding road distances.

### 8. Priority Queue

A priority queue is used in Dijkstra's algorithm to process the location with the smallest known distance first.

A min-heap is used for efficient shortest-path calculation.

### 9. Dijkstra's Shortest Path Algorithm

Dijkstra's algorithm calculates the shortest delivery route between a restaurant and a customer's location.

The algorithm:

- Creates a weighted graph.
- Initializes distances.
- Uses a priority queue.
- Updates distances when a shorter path is found.
- Reconstructs the shortest route.

### 10. Linear Searching

Linear searching is used to find specific records, such as customers, restaurants and food items, by checking elements one by one.

### 11. Maps and Pairs

- `map` is used for storing and retrieving request parameters.
- `pair` is used to store related values such as neighbouring locations and distances.

### 12. Other C++ Concepts

- Constructors and member functions
- Access specifiers (`private` and `public`)
- Functions and modular programming
- Strings and string manipulation
- Pointers and references
- Conditional statements and loops
- Input/output streams
- File handling using `ifstream` and `ofstream`
- STL algorithms such as `reverse()`
- Basic socket programming

---

### How the System Works

1. The user interacts with the browser-based interface.
2. JavaScript sends requests to the local C++ server.
3. The C++ backend processes the requested operation.
4. Data structures and algorithms are used where required.
5. Application records are read from or saved to text files.
6. The C++ backend sends a JSON response.
7. The frontend updates the interface according to the response.

---

## 👥 Team Members & Contributions

### Member 1 – Customer and Order Module

**Responsibilities:**

- Customer registration.
- Viewing and managing customer details.
- Food selection and cart management.
- Bill calculation.
- Order placement.

**Concepts Used:**

- Classes and Objects
- Encapsulation
- Member Functions
- Vectors
- Strings
- Linear Searching
- File Handling

**Status:** Implemented.

### Member 2 – Restaurant and Food Module

**Responsibilities:**

- Managing restaurant details.
- Displaying restaurant information.
- Managing food menus and prices.
- Restaurant and food selection.

**Concepts Used:**

- Classes and Objects
- Encapsulation
- Member Functions
- Vectors
- Strings
- File Handling

**Status:** Implemented.

### Member 3 – Route Optimization Module

**Responsibilities:**

- Representing locations and roads using a weighted graph.
- Creating an adjacency list.
- Calculating the shortest delivery route.
- Determining delivery distance and estimated time.

**Concepts Used:**

- Graphs
- Weighted Graphs
- Adjacency Lists
- Priority Queue
- Dijkstra's Algorithm
- Vectors and Pairs

**Status:** Implemented.

### Member 4 – Delivery and Integration Module

**Responsibilities:**

- Managing pending orders using a queue.
- Maintaining order delivery statuses.
- Integrating different project modules.
- Connecting the frontend with the C++ backend.
- Developing and integrating the user interface.
- Testing the complete application.

**Concepts Used:**

- Classes and Objects
- Encapsulation
- Queue (FIFO)
- Member Functions
- C++ STL
- Socket Programming
- Client-Server Communication

**Status:** Implemented.

---

## 💾 File-Based Storage

Path2Plate uses text files for storing application data instead of an external database.

The application automatically creates a `data/` directory when it runs for the first time.

The following files are used:

| **File**          | **Stored Information**                      |
| ----------------- | ------------------------------------------- |
| `locations.txt`   | Delivery location details                   |
| `roads.txt`       | Roads and distances between locations       |
| `restaurants.txt` | Restaurant information                      |
| `menu.txt`        | Food items and prices                       |
| `customers.txt`   | Registered customer details                 |
| `orders.txt`      | Order details, billing, routes and statuses |

### File Handling Functions

- `ifstream` is used to read data from files.
- `ofstream` is used to write and update data.
- `readRows()` reads records from text files.
- `writeRows()` writes records into text files.
- `loadAll()` loads stored information into vectors.

This provides basic data persistence between application runs.

---

## ⚙️ Installation and Execution

### Requirements

- Windows, Linux or macOS.
- GCC compiler with C++14 support.
- MinGW/MSYS2 UCRT64 on Windows.
- A modern web browser such as Chrome or Edge.

### Windows Setup

1. Clone or download this repository.
2. Extract the project folder.
3. Install MSYS2 with the UCRT64 GCC compiler if not already installed.
4. Open the project folder.
5. Double-click `build_and_run.bat`.
6. Wait until the C++ server starts.
7. Open your browser and visit:

   `http://localhost:8000`
8. Keep the terminal window running while using the application.
9. Press `Ctrl + C` in the terminal to stop the server.

### Manual Compilation (Windows)

If compiling manually using MinGW:

```
g++ -std=c++14 Path2Plate.cpp -o Path2Plate.exe -lws2_32
```

Run:

```
Path2Plate.exe
```

### Linux / macOS Setup

Compile the application:

```
g++ -std=c++14 Path2Plate.cpp -o Path2Plate
```

Run:

```
./Path2Plate
```

Open:

`http://localhost:8000`

**Important:** Run the executable from the project directory because the application loads `index.html` and creates its `data/` directory relative to the current working directory.

---

## 🔐 Admin Access

The project includes an admin interface for demonstration purposes.

**Default demonstration PIN:** `1234`

This PIN is intended only for classroom demonstration and is not suitable for production security.

---

## ⚠️ Project Limitations

Path2Plate is an educational project developed to demonstrate C++ programming and Data Structures and Algorithms.

Current limitations include:

- No external database or DBMS integration.
- No real payment gateway.
- No real-time GPS tracking.
- No OTP-based authentication.
- No real delivery partner integration.
- Admin authentication uses a demonstration PIN.
- Restaurant favourites are stored in browser local storage on the current device.
- Delivery tracking animations are simulated.

---

## 🔮 Future Scope

The project can be improved further by adding:

- Database integration using MySQL or another DBMS.
- Real payment gateway integration.
- Secure authentication and authorization.
- OTP-based login.
- Real-time GPS delivery tracking.
- Improved admin dashboard.
- Cloud-based data storage.
- Better order management and notifications.

---

## 🎯 Project Objective

The main objective of Path2Plate is to apply C++ programming concepts and Data Structures and Algorithms to a practical real-world problem.

Through this project, we demonstrate how classes, objects, encapsulation, vectors, queues, graphs, priority queues, file handling, searching and Dijkstra's shortest-path algorithm can be used together to build a functional food ordering and delivery application.

**Path2Plate – From Restaurant to Your Plate!** 🍕
