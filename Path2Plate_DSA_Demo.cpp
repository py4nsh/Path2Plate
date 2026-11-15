#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <utility>
#include <limits>
#include <iomanip>
using namespace std;

class Customer {
private:
    int id;
    string name;
    int location;
public:
    Customer(int i, string n, int l) : id(i), name(n), location(l) {}
    int getId() const { return id; }
    string getName() const { return name; }
    int getLocation() const { return location; }
    void display() const {
        cout << "Customer ID: " << id << " | Name: " << name
             << " | Delivery Location: " << location << "\n";
    }
};

class Restaurant {
private:
    int id;
    string name;
    int location;
public:
    Restaurant(int i, string n, int l) : id(i), name(n), location(l) {}
    int getId() const { return id; }
    string getName() const { return name; }
    int getLocation() const { return location; }
    void display() const {
        cout << id << ". " << name << " (Location " << location << ")\n";
    }
};

class FoodItem {
private:
    int id;
    int restaurantId;
    string name;
    double price;
public:
    FoodItem(int i, int r, string n, double p)
        : id(i), restaurantId(r), name(n), price(p) {}
    int getId() const { return id; }
    int getRestaurantId() const { return restaurantId; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    void display() const {
        cout << id << ". " << name << " - Rs. " << fixed << setprecision(2) << price << "\n";
    }
};

class CartItem {
private:
    int foodId;
    int quantity;
public:
    CartItem(int f, int q) : foodId(f), quantity(q) {}
    int getFoodId() const { return foodId; }
    int getQuantity() const { return quantity; }
};

class Order {
private:
    int id;
    int customerId;
    int restaurantId;
    vector<CartItem> items;
    double subtotal;
    string paymentMethod;
    string status;
    int distance;
public:
    Order(int i, int c, int r, const vector<CartItem>& cart, double total,
          string payment, int d)
        : id(i), customerId(c), restaurantId(r), items(cart), subtotal(total),
          paymentMethod(payment), status("Placed"), distance(d) {}
    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    int getRestaurantId() const { return restaurantId; }
    double getSubtotal() const { return subtotal; }
    string getStatus() const { return status; }
    int getDistance() const { return distance; }
    void updateStatus() {
        if (status == "Placed") status = "Preparing";
        else if (status == "Preparing") status = "Out for Delivery";
        else if (status == "Out for Delivery") status = "Delivered";
    }
    bool isDelivered() const { return status == "Delivered"; }
    void display() const {
        cout << "Order #" << id << " | Customer: " << customerId
             << " | Restaurant: " << restaurantId
             << " | Subtotal: Rs. " << fixed << setprecision(2) << subtotal
             << " | Payment: " << paymentMethod
             << " | Distance: " << distance << " km"
             << " | Status: " << status << "\n";
    }
};

vector<Customer> customers;
vector<Restaurant> restaurants;
vector<FoodItem> menuItems;
vector<Order> orders;
queue<int> pendingOrders;

int findCustomerIndex(int id) {
    for (int i = 0; i < (int)customers.size(); i++)
        if (customers[i].getId() == id) return i;
    return -1;
}

int findRestaurantIndex(int id) {
    for (int i = 0; i < (int)restaurants.size(); i++)
        if (restaurants[i].getId() == id) return i;
    return -1;
}

int findFoodIndex(int id, int restaurantId) {
    for (int i = 0; i < (int)menuItems.size(); i++)
        if (menuItems[i].getId() == id && menuItems[i].getRestaurantId() == restaurantId) return i;
    return -1;
}

// MEMBER 1 - CUSTOMER AND ORDER MODULE
void registerCustomer() {
    int id, location;
    string name;
    cout << "Enter customer ID: ";
    cin >> id;
    if (findCustomerIndex(id) != -1) {
        cout << "This customer ID already exists.\n";
        return;
    }
    cout << "Enter customer name (one word): ";
    cin >> name;
    cout << "Enter delivery location ID (0-5): ";
    cin >> location;
    if (location < 0 || location >= 6) {
        cout << "Invalid location.\n";
        return;
    }
    customers.push_back(Customer(id, name, location));
    cout << "Customer registered successfully.\n";
}

void viewCustomers() {
    if (customers.empty()) {
        cout << "No customers registered yet.\n";
        return;
    }
    for (int i = 0; i < (int)customers.size(); i++) customers[i].display();
}

double calculateBill(const vector<CartItem>& cart, int restaurantId) {
    double total = 0;
    for (int i = 0; i < (int)cart.size(); i++) {
        int index = findFoodIndex(cart[i].getFoodId(), restaurantId);
        if (index != -1)
            total += menuItems[index].getPrice() * cart[i].getQuantity();
    }
    return total;
}

// MEMBER 2 - RESTAURANT AND FOOD MODULE
void loadRestaurantData() {
    restaurants.push_back(Restaurant(1, "Food Corner", 0));
    restaurants.push_back(Restaurant(2, "Pizza Point", 3));
    restaurants.push_back(Restaurant(3, "Spice House", 4));

    menuItems.push_back(FoodItem(101, 1, "Burger", 120));
    menuItems.push_back(FoodItem(102, 1, "Sandwich", 90));
    menuItems.push_back(FoodItem(103, 1, "Fries", 70));
    menuItems.push_back(FoodItem(201, 2, "Pizza", 250));
    menuItems.push_back(FoodItem(202, 2, "Pasta", 180));
    menuItems.push_back(FoodItem(301, 3, "Paneer Roll", 140));
    menuItems.push_back(FoodItem(302, 3, "Thali", 190));
}

void displayRestaurants() {
    cout << "\nAvailable Restaurants\n";
    for (int i = 0; i < (int)restaurants.size(); i++) restaurants[i].display();
}

void displayMenuForRestaurant(int restaurantId) {
    bool found = false;
    cout << "\nMenu\n";
    for (int i = 0; i < (int)menuItems.size(); i++) {
        if (menuItems[i].getRestaurantId() == restaurantId) {
            menuItems[i].display();
            found = true;
        }
    }
    if (!found) cout << "No food items available.\n";
}

// MEMBER 3 - ROUTE OPTIMIZATION MODULE
const int LOCATION_COUNT = 6;
string locationNames[LOCATION_COUNT] = {
    "College", "Market", "Bus Stand", "City Centre", "Hospital", "Residential Area"
};
vector<pair<int, int> > roadMap[LOCATION_COUNT];

void addRoad(int from, int to, int distance) {
    roadMap[from].push_back(make_pair(to, distance));
    roadMap[to].push_back(make_pair(from, distance));
}

void createRoadMap() {
    addRoad(0, 1, 4);
    addRoad(0, 2, 2);
    addRoad(1, 2, 1);
    addRoad(1, 3, 5);
    addRoad(2, 3, 8);
    addRoad(3, 4, 3);
    addRoad(4, 5, 2);
    addRoad(2, 5, 10);
}

int findShortestRoute(int start, int destination, bool showRoute) {
    const int INF = numeric_limits<int>::max();
    vector<int> distance(LOCATION_COUNT, INF);
    vector<int> parent(LOCATION_COUNT, -1);
    priority_queue<pair<int, int>, vector<pair<int, int> >,
                   greater<pair<int, int> > > pq;

    distance[start] = 0;
    pq.push(make_pair(0, start));

    while (!pq.empty()) {
        int currentDistance = pq.top().first;
        int current = pq.top().second;
        pq.pop();
        if (currentDistance != distance[current]) continue;

        for (int i = 0; i < (int)roadMap[current].size(); i++) {
            int next = roadMap[current][i].first;
            int weight = roadMap[current][i].second;
            if (distance[current] + weight < distance[next]) {
                distance[next] = distance[current] + weight;
                parent[next] = current;
                pq.push(make_pair(distance[next], next));
            }
        }
    }

    if (distance[destination] == INF) {
        if (showRoute) cout << "No route is available between these locations.\n";
        return -1;
    }

    if (showRoute) {
        vector<int> path;
        for (int at = destination; at != -1; at = parent[at]) path.push_back(at);
        cout << "Shortest route: ";
        for (int i = (int)path.size() - 1; i >= 0; i--) {
            cout << locationNames[path[i]];
            if (i != 0) cout << " -> ";
        }
        cout << "\nTotal distance: " << distance[destination] << " km\n";
    }
    return distance[destination];
}

void displayLocations() {
    cout << "\nDelivery Locations\n";
    for (int i = 0; i < LOCATION_COUNT; i++)
        cout << i << ". " << locationNames[i] << "\n";
}

// MEMBER 4 - DELIVERY AND INTEGRATION MODULE
void placeOrder() {
    int customerId, restaurantId;
    cout << "Enter customer ID: ";
    cin >> customerId;
    int customerIndex = findCustomerIndex(customerId);
    if (customerIndex == -1) {
        cout << "Customer not found. Register first.\n";
        return;
    }

    displayRestaurants();
    cout << "Select restaurant ID: ";
    cin >> restaurantId;
    int restaurantIndex = findRestaurantIndex(restaurantId);
    if (restaurantIndex == -1) {
        cout << "Invalid restaurant ID.\n";
        return;
    }

    displayMenuForRestaurant(restaurantId);
    vector<CartItem> cart;
    char more = 'y';
    while (more == 'y' || more == 'Y') {
        int foodId, quantity;
        cout << "Enter food ID and quantity: ";
        cin >> foodId >> quantity;
        if (findFoodIndex(foodId, restaurantId) == -1 || quantity <= 0) {
            cout << "Invalid food ID or quantity. Try again.\n";
        } else {
            cart.push_back(CartItem(foodId, quantity));
        }
        cout << "Add another item? (y/n): ";
        cin >> more;
    }

    if (cart.empty()) {
        cout << "Cart is empty. Order not placed.\n";
        return;
    }

    double subtotal = calculateBill(cart, restaurantId);
    int start = restaurants[restaurantIndex].getLocation();
    int destination = customers[customerIndex].getLocation();
    int distance = findShortestRoute(start, destination, false);
    if (distance == -1) {
        cout << "Order cannot be placed because no delivery route exists.\n";
        return;
    }

    int paymentChoice;
    string paymentMethod;
    cout << "Payment Method\n1. Cash on Delivery\n2. UPI Demo\n3. Card Demo\nChoose: ";
    cin >> paymentChoice;
    if (paymentChoice == 1) paymentMethod = "Cash on Delivery";
    else if (paymentChoice == 2) paymentMethod = "UPI Demo";
    else if (paymentChoice == 3) paymentMethod = "Card Demo";
    else {
        cout << "Invalid payment option. Order not placed.\n";
        return;
    }

    int orderId = (int)orders.size() + 1;
    orders.push_back(Order(orderId, customerId, restaurantId, cart,
                           subtotal, paymentMethod, distance));
    pendingOrders.push(orderId);

    cout << "\nOrder placed successfully!\n";
    cout << "Items subtotal: Rs. " << fixed << setprecision(2) << subtotal << "\n";
    cout << "Shortest delivery distance: " << distance << " km\n";
    cout << "Estimated travel time: " << distance * 3 << " minutes (demo estimate)\n";
    cout << "Payment selected: " << paymentMethod << "\n";
    cout << "Order ID: " << orderId << "\n";
}

void processNextOrder() {
    if (pendingOrders.empty()) {
        cout << "No pending orders.\n";
        return;
    }
    int orderId = pendingOrders.front();
    pendingOrders.pop();
    if (orderId <= 0 || orderId > (int)orders.size()) return;

    Order& order = orders[orderId - 1];
    order.updateStatus();
    cout << "Order " << order.getId() << " status: " << order.getStatus() << "\n";
    if (!order.isDelivered()) pendingOrders.push(orderId);
}

void displayOrders() {
    if (orders.empty()) {
        cout << "No orders have been placed yet.\n";
        return;
    }
    cout << "\nAll Orders\n";
    for (int i = 0; i < (int)orders.size(); i++) orders[i].display();
}

void showBillDetails() {
    if (orders.empty()) {
        cout << "No bill is available yet.\n";
        return;
    }
    int id;
    cout << "Enter order ID: ";
    cin >> id;
    if (id <= 0 || id > (int)orders.size()) {
        cout << "Invalid order ID.\n";
        return;
    }
    cout << "Bill subtotal: Rs. " << fixed << setprecision(2)
         << orders[id - 1].getSubtotal() << "\n";
    cout << "This academic demo does not process real payments or add online transaction charges.\n";
}

int main() {
    loadRestaurantData();
    createRoadMap();
    int choice;
    do {
        cout << "\n========== PATH2PLATE | C++ AND DSA =========="
             << "\n1. Register Customer"
             << "\n2. View Customers"
             << "\n3. Display Restaurants"
             << "\n4. View Restaurant Menu"
             << "\n5. Place Order"
             << "\n6. Process Next Delivery"
             << "\n7. Display All Orders"
             << "\n8. Find Shortest Route"
             << "\n9. View Bill"
             << "\n10. Display Delivery Locations"
             << "\n0. Exit"
             << "\nEnter choice: ";
        cin >> choice;

        if (choice == 1) registerCustomer();
        else if (choice == 2) viewCustomers();
        else if (choice == 3) displayRestaurants();
        else if (choice == 4) {
            int id;
            displayRestaurants();
            cout << "Enter restaurant ID: ";
            cin >> id;
            if (findRestaurantIndex(id) == -1) cout << "Invalid restaurant ID.\n";
            else displayMenuForRestaurant(id);
        }
        else if (choice == 5) placeOrder();
        else if (choice == 6) processNextOrder();
        else if (choice == 7) displayOrders();
        else if (choice == 8) {
            int start, destination;
            displayLocations();
            cout << "Enter starting and destination location IDs: ";
            cin >> start >> destination;
            if (start < 0 || start >= LOCATION_COUNT ||
                destination < 0 || destination >= LOCATION_COUNT)
                cout << "Invalid location ID.\n";
            else findShortestRoute(start, destination, true);
        }
        else if (choice == 9) showBillDetails();
        else if (choice == 10) displayLocations();
        else if (choice != 0) cout << "Invalid menu choice.\n";
    } while (choice != 0);

    cout << "Thank you for using Path2Plate.\n";
    return 0;
}
