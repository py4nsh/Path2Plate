/*
 * Path2Plate - Online Food Delivery System with Integrated Route Optimizer
 * Team Code_crew | B.Tech CSE | Graphic Era Hill University
 *
 * Written in C++ (standard library only). The program contains a tiny web server
 * so the user interface (index.html) can talk to our C++ code.
 *
 * BUILD (Windows, MinGW / VS Code):  g++ Path2Plate.cpp -o Path2Plate.exe -lws2_32
 * BUILD (Linux / Mac):               g++ Path2Plate.cpp -o Path2Plate
 * RUN (from this folder):            Path2Plate.exe     ->  http://localhost:8000
 * Admin PIN: 1234
 *
 * C++ / DSA concepts used:
 *   classes/structs, vectors, functions, string handling, file handling,
 *   queue (pending orders), graph (adjacency list), priority_queue, Dijkstra.
 *
 * "Database" = text files in the data/ folder (fields separated by |):
 *   locations.txt   id|name|x|y
 *   roads.txt       locationA|locationB|km
 *   restaurants.txt id|name|location|cuisine|emoji
 *   menu.txt        id|restaurant|name|price|V/N
 *   customers.txt   id|name|phone|location
 *   orders.txt      number|customer|restaurant|items|subtotal|delivery|gst|total|status|km|path|time
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

// ---------- sockets (Windows or Linux/Mac) ----------
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <direct.h>
  #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
  #endif
  typedef SOCKET sock_t;
  #define CLOSESOCK closesocket
#else
  #include <sys/socket.h>
  #include <sys/stat.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int sock_t;
  #define INVALID_SOCKET (-1)
  #define CLOSESOCK close
#endif

const string DATA_DIR = "data";
const string ADMIN_PIN = "1234";
const int PORT = 8000;
const string STATUS_FLOW[4] = {"Placed", "Preparing", "Out for Delivery", "Delivered"};

// =====================================================================
//  DATA CLASSES
// =====================================================================
struct Location   { int id; string name; int x, y; };
struct Road       { int a, b; double km; };
struct Restaurant { int id; string name; int loc; string cuisine, emoji; };
struct FoodItem   { int id; int rid; string name; int price; bool veg; };
struct Customer   { int id; string name, phone; int loc; };
struct OrderLine  { string name; int price, qty, amount; };
struct Order {
    int num; int cid, rid;
    vector<OrderLine> items;
    double subtotal, delivery, gst, total, km;
    string status;
    string paymentMethod, paymentStatus;
    vector<int> path;
    string time;
};
struct RouteResult { bool found; double km; vector<int> path; };
struct Bill {
    vector<OrderLine> items;
    double subtotal, delivery, gst, total, km;
    vector<int> path;
    int eta;
};

// Our in-memory copy of the files
vector<Location> locations;
vector<Road> roads;
vector<Restaurant> restaurants;
vector<FoodItem> menu;
vector<Customer> customers;
vector<Order> orders;

// =====================================================================
//  SMALL HELPERS
// =====================================================================
vector<string> split(const string& s, char sep) {
    vector<string> parts;
    string cur;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == sep) { parts.push_back(cur); cur = ""; }
        else cur += s[i];
    }
    parts.push_back(cur);
    return parts;
}

// "|" is our file separator, so remove it (and a few other symbols) from user input
string cleanText(string s) {
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '|' || c == '~' || c == ';') c = '/';
        if (c == '\n' || c == '\r' || c == '\t') c = ' ';
        out += c;
    }
    size_t a = out.find_first_not_of(' ');
    if (a == string::npos) return "";
    size_t b = out.find_last_not_of(' ');
    return out.substr(a, b - a + 1);
}

string toStr(int n) { ostringstream o; o << n; return o.str(); }

// numbers for JSON: whole numbers without decimals, otherwise 2 decimals
string money(double v) {
    char buf[64];
    if (fabs(v - floor(v)) < 0.0001) snprintf(buf, sizeof(buf), "%.0f", v);
    else snprintf(buf, sizeof(buf), "%.2f", v);
    return buf;
}
string km1(double v) { char buf[64]; snprintf(buf, sizeof(buf), "%.1f", v); return buf; }

string jstr(const string& s) {   // make a safe JSON string
    string o = "\"";
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '"') o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c == '\n') o += "\\n";
        else if ((unsigned char)c < 32) o += ' ';
        else o += c;
    }
    return o + "\"";
}

int etaMinutes(double km) { return 15 + (int)floor(km * 4 + 0.5); }

string nowString() {
    char buf[64];
    time_t t = time(NULL);
    strftime(buf, sizeof(buf), "%d %b %H:%M", localtime(&t));
    return buf;
}

// =====================================================================
//  FILE HANDLING  (our database)
// =====================================================================
vector<vector<string> > readRows(const string& name) {
    vector<vector<string> > rows;
    ifstream f((DATA_DIR + "/" + name + ".txt").c_str());
    string line;
    while (getline(f, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.empty()) continue;
        rows.push_back(split(line, '|'));
    }
    return rows;
}

void writeRows(const string& name, const vector<vector<string> >& rows) {
    ofstream f((DATA_DIR + "/" + name + ".txt").c_str());
    for (size_t i = 0; i < rows.size(); i++) {
        for (size_t j = 0; j < rows[i].size(); j++) {
            if (j) f << "|";
            f << rows[i][j];
        }
        f << "\n";
    }
}

bool fileExists(const string& name) {
    ifstream f((DATA_DIR + "/" + name + ".txt").c_str());
    return f.good();
}

void makeDataFolder() {
#ifdef _WIN32
    _mkdir(DATA_DIR.c_str());
#else
    mkdir(DATA_DIR.c_str(), 0755);
#endif
}

// first-run data
void createStartingFiles() {
    makeDataFolder();
    if (!fileExists("locations")) {
        const char* d[][4] = {
            {"1","Clock Tower","330","200"}, {"2","Paltan Bazaar","410","140"},
            {"3","ISBT","540","90"}, {"4","Rajpur Road","250","100"},
            {"5","Ballupur Chowk","140","190"}, {"6","Prem Nagar","80","290"},
            {"7","Race Course","330","310"}, {"8","Sahastradhara Road","570","240"},
            {"9","Graphic Era University","470","380"}, {"10","Clement Town","240","400"}};
        vector<vector<string> > rows;
        for (int i = 0; i < 10; i++) rows.push_back(vector<string>(d[i], d[i] + 4));
        writeRows("locations", rows);
    }
    if (!fileExists("roads")) {
        const char* d[][3] = {
            {"1","2","1.2"}, {"1","4","2.5"}, {"1","7","2.0"}, {"1","5","3.0"},
            {"2","3","1.5"}, {"2","8","4.0"}, {"3","8","3.5"}, {"4","5","2.8"},
            {"5","6","2.2"}, {"6","10","5.5"}, {"6","7","4.0"}, {"7","9","3.5"},
            {"7","10","4.5"}, {"9","10","5.0"}, {"9","8","6.0"}, {"7","8","5.0"}};
        vector<vector<string> > rows;
        for (int i = 0; i < 16; i++) rows.push_back(vector<string>(d[i], d[i] + 3));
        writeRows("roads", rows);
    }
    if (!fileExists("restaurants")) {
        const char* d[][5] = {
            {"1","Spice Route","4","North Indian","\xF0\x9F\x8D\x9B"},
            {"2","Pizza Junction","2","Italian","\xF0\x9F\x8D\x95"},
            {"3","Momo Magic","5","Chinese & Tibetan","\xF0\x9F\xA5\x9F"},
            {"4","Campus Bites","9","Fast Food","\xF0\x9F\x8D\x94"}};
        vector<vector<string> > rows;
        for (int i = 0; i < 4; i++) rows.push_back(vector<string>(d[i], d[i] + 5));
        writeRows("restaurants", rows);
    }
    if (!fileExists("menu")) {
        const char* d[][5] = {
            {"1","1","Paneer Butter Masala","220","V"}, {"2","1","Butter Chicken","280","N"},
            {"3","1","Dal Makhani","180","V"}, {"4","1","Butter Naan","35","V"},
            {"5","1","Jeera Rice","120","V"}, {"6","2","Margherita Pizza","199","V"},
            {"7","2","Farmhouse Pizza","299","V"}, {"8","2","Chicken Tikka Pizza","349","N"},
            {"9","2","Garlic Bread","99","V"}, {"10","3","Veg Steamed Momos","80","V"},
            {"11","3","Chicken Fried Momos","110","N"}, {"12","3","Thukpa","130","N"},
            {"13","3","Chilli Paneer","160","V"}, {"14","4","Veg Burger","70","V"},
            {"15","4","Chicken Burger","110","N"}, {"16","4","French Fries","60","V"},
            {"17","4","Cold Coffee","70","V"}, {"18","4","Maggi","50","V"}};
        vector<vector<string> > rows;
        for (int i = 0; i < 18; i++) rows.push_back(vector<string>(d[i], d[i] + 5));
        writeRows("menu", rows);
    }
    if (!fileExists("customers")) writeRows("customers", vector<vector<string> >());
    if (!fileExists("orders")) writeRows("orders", vector<vector<string> >());
}

// ---------- load from files into vectors ----------
void loadAll() {
    locations.clear(); roads.clear(); restaurants.clear();
    menu.clear(); customers.clear(); orders.clear();
    vector<vector<string> > r;

    r = readRows("locations");
    for (size_t i = 0; i < r.size(); i++) {
        Location l = { atoi(r[i][0].c_str()), r[i][1], atoi(r[i][2].c_str()), atoi(r[i][3].c_str()) };
        locations.push_back(l);
    }
    r = readRows("roads");
    for (size_t i = 0; i < r.size(); i++) {
        Road d = { atoi(r[i][0].c_str()), atoi(r[i][1].c_str()), atof(r[i][2].c_str()) };
        roads.push_back(d);
    }
    r = readRows("restaurants");
    for (size_t i = 0; i < r.size(); i++) {
        Restaurant x = { atoi(r[i][0].c_str()), r[i][1], atoi(r[i][2].c_str()), r[i][3], r[i][4] };
        restaurants.push_back(x);
    }
    r = readRows("menu");
    for (size_t i = 0; i < r.size(); i++) {
        FoodItem m = { atoi(r[i][0].c_str()), atoi(r[i][1].c_str()), r[i][2], atoi(r[i][3].c_str()), r[i][4] == "V" };
        menu.push_back(m);
    }
    r = readRows("customers");
    for (size_t i = 0; i < r.size(); i++) {
        Customer c = { atoi(r[i][0].c_str()), r[i][1], r[i][2], atoi(r[i][3].c_str()) };
        customers.push_back(c);
    }
    r = readRows("orders");
    for (size_t i = 0; i < r.size(); i++) {
        Order o;
        o.num = atoi(r[i][0].c_str()); o.cid = atoi(r[i][1].c_str()); o.rid = atoi(r[i][2].c_str());
        vector<string> lines = split(r[i][3], ';');
        for (size_t j = 0; j < lines.size(); j++) {
            vector<string> p = split(lines[j], '~');
            if (p.size() < 3) continue;
            OrderLine ol = { p[0], atoi(p[1].c_str()), atoi(p[2].c_str()), 0 };
            ol.amount = ol.price * ol.qty;
            o.items.push_back(ol);
        }
        o.subtotal = atof(r[i][4].c_str()); o.delivery = atof(r[i][5].c_str());
        o.gst = atof(r[i][6].c_str());      o.total = atof(r[i][7].c_str());
        o.status = r[i][8]; o.km = atof(r[i][9].c_str());
        vector<string> p = split(r[i][10], '>');
        for (size_t j = 0; j < p.size(); j++) o.path.push_back(atoi(p[j].c_str()));
        o.time = r[i][11];
        o.paymentMethod = r[i].size() > 12 ? r[i][12] : "Cash on Delivery";
        o.paymentStatus = r[i].size() > 13 ? r[i][13] : "Pay on delivery";
        orders.push_back(o);
    }
}

// ---------- save vectors back to files ----------
void saveMenu() {
    vector<vector<string> > rows;
    for (size_t i = 0; i < menu.size(); i++) {
        vector<string> row;
        row.push_back(toStr(menu[i].id)); row.push_back(toStr(menu[i].rid)); row.push_back(menu[i].name);
        row.push_back(toStr(menu[i].price)); row.push_back(menu[i].veg ? "V" : "N");
        rows.push_back(row);
    }
    writeRows("menu", rows);
}
void saveCustomers() {
    vector<vector<string> > rows;
    for (size_t i = 0; i < customers.size(); i++) {
        vector<string> row;
        row.push_back(toStr(customers[i].id)); row.push_back(customers[i].name);
        row.push_back(customers[i].phone); row.push_back(toStr(customers[i].loc));
        rows.push_back(row);
    }
    writeRows("customers", rows);
}
void saveOrders() {
    vector<vector<string> > rows;
    for (size_t i = 0; i < orders.size(); i++) {
        const Order& o = orders[i];
        string items, path;
        for (size_t j = 0; j < o.items.size(); j++) {
            if (j) items += ";";
            items += o.items[j].name + "~" + toStr(o.items[j].price) + "~" + toStr(o.items[j].qty);
        }
        for (size_t j = 0; j < o.path.size(); j++) { if (j) path += ">"; path += toStr(o.path[j]); }
        vector<string> row;
        row.push_back(toStr(o.num)); row.push_back(toStr(o.cid)); row.push_back(toStr(o.rid));
        row.push_back(items); row.push_back(money(o.subtotal)); row.push_back(money(o.delivery));
        row.push_back(money(o.gst)); row.push_back(money(o.total)); row.push_back(o.status);
        row.push_back(km1(o.km)); row.push_back(path); row.push_back(o.time);
        row.push_back(o.paymentMethod); row.push_back(o.paymentStatus);
        rows.push_back(row);
    }
    writeRows("orders", rows);
}

// =====================================================================
//  LOOKUP FUNCTIONS
// =====================================================================
string locationName(int id) {
    for (size_t i = 0; i < locations.size(); i++) if (locations[i].id == id) return locations[i].name;
    return "?";
}
bool locationExists(int id) {
    for (size_t i = 0; i < locations.size(); i++) if (locations[i].id == id) return true;
    return false;
}
const Restaurant* findRestaurant(int id) {
    for (size_t i = 0; i < restaurants.size(); i++) if (restaurants[i].id == id) return &restaurants[i];
    return NULL;
}
Customer* findCustomer(int id) {
    for (size_t i = 0; i < customers.size(); i++) if (customers[i].id == id) return &customers[i];
    return NULL;
}

// =====================================================================
//  MODULE 3 - ROUTE OPTIMIZER  (weighted graph + Dijkstra + priority queue)
// =====================================================================
RouteResult dijkstra(int src, int dst) {
    RouteResult res;
    res.found = false; res.km = 0;

    int n = 0;
    for (size_t i = 0; i < locations.size(); i++) n = max(n, locations[i].id);
    if (!locationExists(src) || !locationExists(dst)) return res;

    // adjacency list: adj[u] = list of (neighbour, distance)
    vector<vector<pair<int, double> > > adj(n + 1);
    for (size_t i = 0; i < roads.size(); i++) {
        adj[roads[i].a].push_back(make_pair(roads[i].b, roads[i].km));
        adj[roads[i].b].push_back(make_pair(roads[i].a, roads[i].km));   // two-way road
    }

    vector<double> dist(n + 1, 1e18);
    vector<int> prev(n + 1, -1);
    // min-heap: smallest distance on top
    priority_queue<pair<double, int>, vector<pair<double, int> >, greater<pair<double, int> > > pq;

    dist[src] = 0;
    pq.push(make_pair(0.0, src));
    while (!pq.empty()) {
        pair<double, int> top = pq.top(); pq.pop();
        double d = top.first; int u = top.second;
        if (d > dist[u]) continue;           // old entry, skip
        if (u == dst) break;
        for (size_t i = 0; i < adj[u].size(); i++) {
            int v = adj[u][i].first; double w = adj[u][i].second;
            if (d + w < dist[v]) {
                dist[v] = d + w;
                prev[v] = u;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
    if (dist[dst] > 1e17) return res;

    // rebuild the path from dst back to src
    int cur = dst;
    while (cur != -1) { res.path.push_back(cur); cur = prev[cur]; }
    reverse(res.path.begin(), res.path.end());
    res.found = true;
    res.km = floor(dist[dst] * 10 + 0.5) / 10;
    return res;
}

// =====================================================================
//  MODULE 1 - CUSTOMER & ORDER
// =====================================================================
bool registerCustomer(const string& nameIn, const string& phoneIn, int loc, Customer& out, string& err) {
    string name = cleanText(nameIn), phone = cleanText(phoneIn);
    if (name.size() < 2) { err = "Please enter your name"; return false; }
    if (phone.size() != 10 || phone.find_first_not_of("0123456789") != string::npos) {
        err = "Phone number must be 10 digits"; return false;
    }
    if (!locationExists(loc)) { err = "Please choose a location"; return false; }
    for (size_t i = 0; i < customers.size(); i++)
        if (customers[i].phone == phone) { err = "This phone number is already registered. Please login."; return false; }
    Customer c;
    c.id = customers.empty() ? 1 : customers.back().id + 1;
    c.name = name; c.phone = phone; c.loc = loc;
    customers.push_back(c);
    saveCustomers();
    out = c;
    return true;
}

// cart text looks like  "1:2,4:3"  (menu item id : quantity)
bool makeBill(int rid, const string& cart, int customerLoc, Bill& b, string& err) {
    b.items.clear(); b.subtotal = 0;
    vector<string> parts = split(cart, ',');
    for (size_t i = 0; i < parts.size(); i++) {
        vector<string> p = split(parts[i], ':');
        if (p.size() != 2) continue;
        int mid = atoi(p[0].c_str()), qty = atoi(p[1].c_str());
        if (qty <= 0) continue;
        bool found = false;
        for (size_t j = 0; j < menu.size(); j++) {
            if (menu[j].id == mid && menu[j].rid == rid) {      // linear search
                OrderLine ol = { menu[j].name, menu[j].price, qty, menu[j].price * qty };
                b.items.push_back(ol);
                b.subtotal += ol.amount;
                found = true;
                break;
            }
        }
        if (!found) { err = "Item not found in this restaurant"; return false; }
    }
    if (b.items.empty()) { err = "Your cart is empty"; return false; }
    const Restaurant* r = findRestaurant(rid);
    if (!r) { err = "Restaurant not found"; return false; }
    RouteResult route = dijkstra(r->loc, customerLoc);
    if (!route.found) { err = "No delivery route available"; return false; }
    b.km = route.km; b.path = route.path;
    b.delivery = floor(20 + 8 * route.km + 0.5);
    b.gst = floor(b.subtotal * 0.05 * 100 + 0.5) / 100;
    b.total = floor((b.subtotal + b.delivery + b.gst) * 100 + 0.5) / 100;
    b.eta = etaMinutes(route.km);
    return true;
}

// =====================================================================
//  MODULE 4 - DELIVERY (order queue + status)
// =====================================================================
queue<int> pendingQueue() {          // FIFO queue of orders waiting to be picked up
    queue<int> q;
    for (size_t i = 0; i < orders.size(); i++)
        if (orders[i].status == "Placed") q.push(orders[i].num);
    return q;
}
Order* findOrder(int num) {
    for (size_t i = 0; i < orders.size(); i++) if (orders[i].num == num) return &orders[i];
    return NULL;
}
void advanceStatus(Order* o) {
    for (int i = 0; i < 3; i++)
        if (o->status == STATUS_FLOW[i]) { o->status = STATUS_FLOW[i + 1]; break; }
    saveOrders();
}

// =====================================================================
//  JSON BUILDERS (text sent to the web page)
// =====================================================================
string pathIdsJson(const vector<int>& p) {
    string s = "[";
    for (size_t i = 0; i < p.size(); i++) { if (i) s += ","; s += jstr(toStr(p[i])); }
    return s + "]";
}
string pathNamesJson(const vector<int>& p) {
    string s = "[";
    for (size_t i = 0; i < p.size(); i++) { if (i) s += ","; s += jstr(locationName(p[i])); }
    return s + "]";
}
string itemsJson(const vector<OrderLine>& items) {
    string s = "[";
    for (size_t i = 0; i < items.size(); i++) {
        if (i) s += ",";
        s += "{\"name\":" + jstr(items[i].name) + ",\"price\":" + toStr(items[i].price) +
             ",\"qty\":" + toStr(items[i].qty) + ",\"amount\":" + toStr(items[i].amount) + "}";
    }
    return s + "]";
}
string customerJson(const Customer& c) {
    return "{\"id\":" + jstr(toStr(c.id)) + ",\"name\":" + jstr(c.name) + ",\"phone\":" + jstr(c.phone) +
           ",\"location_id\":" + jstr(toStr(c.loc)) + ",\"location\":" + jstr(locationName(c.loc)) + "}";
}
string billJson(const Bill& b) {
    return "{\"items\":" + itemsJson(b.items) + ",\"subtotal\":" + money(b.subtotal) +
           ",\"delivery\":" + money(b.delivery) + ",\"gst\":" + money(b.gst) + ",\"total\":" + money(b.total) +
           ",\"distance\":" + km1(b.km) + ",\"path\":" + pathIdsJson(b.path) +
           ",\"path_names\":" + pathNamesJson(b.path) + ",\"eta\":" + toStr(b.eta) + "}";
}
string orderJson(const Order& o) {
    Customer* c = findCustomer(o.cid);
    const Restaurant* r = findRestaurant(o.rid);
    return "{\"id\":" + jstr("ORD" + toStr(o.num)) +
           ",\"customer\":" + jstr(c ? c->name : "?") +
           ",\"customer_location\":" + jstr(c ? locationName(c->loc) : "?") +
           ",\"restaurant\":" + jstr(r ? r->name : "?") +
           ",\"items\":" + itemsJson(o.items) + ",\"subtotal\":" + money(o.subtotal) +
           ",\"delivery\":" + money(o.delivery) + ",\"gst\":" + money(o.gst) + ",\"total\":" + money(o.total) +
           ",\"status\":" + jstr(o.status) + ",\"distance\":" + km1(o.km) +
           ",\"path\":" + pathIdsJson(o.path) + ",\"path_names\":" + pathNamesJson(o.path) +
           ",\"eta\":" + toStr(etaMinutes(o.km)) + ",\"time\":" + jstr(o.time) +
           ",\"payment_method\":" + jstr(o.paymentMethod) + ",\"payment_status\":" + jstr(o.paymentStatus) + "}";
}

// =====================================================================
//  REQUEST HANDLING (the "integration" layer)
// =====================================================================
struct Response { int code; string type; string body; };

Response jsonOk(const string& body) { Response r = {200, "application/json; charset=utf-8", body}; return r; }
Response jsonErr(int code, const string& msg) {
    Response r = {code, "application/json; charset=utf-8", "{\"error\":" + jstr(msg) + "}"}; return r;
}

string param(const map<string, string>& P, const string& key) {
    map<string, string>::const_iterator it = P.find(key);
    return it == P.end() ? "" : it->second;
}

string readFileText(const string& path) {
    ifstream f(path.c_str(), ios::binary);
    stringstream ss; ss << f.rdbuf();
    return ss.str();
}

Response handle(const string& path, const map<string, string>& P, const string& pin) {
    bool isAdmin = (pin == ADMIN_PIN);

    // ----- web page -----
    if (path == "/" || path == "/index.html") {
        string html = readFileText("index.html");
        if (html.empty()) return jsonErr(404, "index.html not found - run the program from its own folder");
        Response r = {200, "text/html; charset=utf-8", html};
        return r;
    }

    // ----- map data -----
    if (path == "/api/locations") {
        string s = "{\"nodes\":[";
        for (size_t i = 0; i < locations.size(); i++) {
            if (i) s += ",";
            s += "{\"id\":" + jstr(toStr(locations[i].id)) + ",\"name\":" + jstr(locations[i].name) +
                 ",\"x\":" + toStr(locations[i].x) + ",\"y\":" + toStr(locations[i].y) + "}";
        }
        s += "],\"edges\":[";
        for (size_t i = 0; i < roads.size(); i++) {
            if (i) s += ",";
            s += "{\"a\":" + jstr(toStr(roads[i].a)) + ",\"b\":" + jstr(toStr(roads[i].b)) +
                 ",\"km\":" + km1(roads[i].km) + "}";
        }
        return jsonOk(s + "]}");
    }
    if (path == "/api/route") {
        RouteResult r = dijkstra(atoi(param(P, "src").c_str()), atoi(param(P, "dst").c_str()));
        if (!r.found) return jsonErr(400, "No route found");
        return jsonOk("{\"distance\":" + km1(r.km) + ",\"path\":" + pathIdsJson(r.path) +
                      ",\"path_names\":" + pathNamesJson(r.path) + "}");
    }

    // ----- Module 2: restaurants and menu -----
    if (path == "/api/restaurants") {
        int loc = atoi(param(P, "loc").c_str());
        string s = "[";
        for (size_t i = 0; i < restaurants.size(); i++) {
            const Restaurant& r = restaurants[i];
            if (i) s += ",";
            s += "{\"id\":" + jstr(toStr(r.id)) + ",\"name\":" + jstr(r.name) +
                 ",\"location\":" + jstr(locationName(r.loc)) + ",\"cuisine\":" + jstr(r.cuisine) +
                 ",\"emoji\":" + jstr(r.emoji);
            if (loc > 0) {
                RouteResult rt = dijkstra(r.loc, loc);
                s += ",\"distance\":" + km1(rt.km) + ",\"eta\":" + toStr(etaMinutes(rt.km));
            }
            s += "}";
        }
        return jsonOk(s + "]");
    }
    if (path == "/api/menu") {
        int rid = atoi(param(P, "rid").c_str());
        string s = "[";
        bool first = true;
        for (size_t i = 0; i < menu.size(); i++) {
            if (menu[i].rid != rid) continue;
            if (!first) s += ",";
            first = false;
            s += "{\"id\":" + jstr(toStr(menu[i].id)) + ",\"name\":" + jstr(menu[i].name) +
                 ",\"price\":" + toStr(menu[i].price) + ",\"veg\":" + (menu[i].veg ? "true" : "false") + "}";
        }
        return jsonOk(s + "]");
    }

    // ----- Module 1: customers and orders -----
    if (path == "/api/register") {
        Customer c; string err;
        if (!registerCustomer(param(P, "name"), param(P, "phone"), atoi(param(P, "location_id").c_str()), c, err))
            return jsonErr(400, err);
        return jsonOk(customerJson(c));
    }
    if (path == "/api/login") {
        string phone = cleanText(param(P, "phone"));
        for (size_t i = 0; i < customers.size(); i++)
            if (customers[i].phone == phone) return jsonOk(customerJson(customers[i]));
        return jsonErr(404, "No account with this phone number. Please register.");
    }
    if (path == "/api/customer") {
        Customer* c = findCustomer(atoi(param(P, "id").c_str()));
        if (!c) return jsonErr(404, "not found");
        return jsonOk(customerJson(*c));
    }
    if (path == "/api/bill" || path == "/api/order") {
        Customer* c = findCustomer(atoi(param(P, "customer_id").c_str()));
        if (!c) return jsonErr(400, "Please login first");
        int rid = atoi(param(P, "restaurant_id").c_str());
        Bill b; string err;
        if (!makeBill(rid, param(P, "cart"), c->loc, b, err)) return jsonErr(400, err);
        if (path == "/api/bill") return jsonOk(billJson(b));

        string payment = param(P, "payment_method");
        if (payment.empty()) payment = "Cash on Delivery";
        if (payment != "Cash on Delivery" && payment != "UPI (Demo)" && payment != "Card (Demo)")
            return jsonErr(400, "Please choose a valid payment method");

        Order o;
        o.num = orders.empty() ? 1001 : orders.back().num + 1;
        o.cid = c->id; o.rid = rid; o.items = b.items;
        o.subtotal = b.subtotal; o.delivery = b.delivery; o.gst = b.gst; o.total = b.total;
        o.status = "Placed"; o.km = b.km; o.path = b.path; o.time = nowString();
        o.paymentMethod = payment;
        o.paymentStatus = payment == "Cash on Delivery" ? "Pay on delivery" : "Demo payment successful";
        orders.push_back(o);
        saveOrders();
        return jsonOk(orderJson(o));
    }
    if (path == "/api/orders") {
        int cid = atoi(param(P, "customer_id").c_str());
        string s = "[";
        bool first = true;
        for (int i = (int)orders.size() - 1; i >= 0; i--) {      // newest first
            if (orders[i].cid != cid) continue;
            if (!first) s += ",";
            first = false;
            s += orderJson(orders[i]);
        }
        return jsonOk(s + "]");
    }

    // ----- Admin (Module 2 + 4) -----
    if (path.compare(0, 11, "/api/admin/") == 0) {
        if (!isAdmin) return jsonErr(403, "Wrong admin PIN");
        string action = path.substr(11);

        if (action == "login") return jsonOk("{\"ok\":true}");

        if (action == "orders") {
            string s = "{\"orders\":[";
            for (int i = (int)orders.size() - 1; i >= 0; i--) {
                s += orderJson(orders[i]);
                if (i > 0) s += ",";
            }
            s += "],\"queue\":[";
            queue<int> q = pendingQueue();
            bool first = true;
            while (!q.empty()) {
                if (!first) s += ",";
                first = false;
                s += jstr("ORD" + toStr(q.front()));
                q.pop();
            }
            return jsonOk(s + "]}");
        }
        if (action == "customers") {
            string s = "[";
            for (size_t i = 0; i < customers.size(); i++) {
                if (i) s += ",";
                s += customerJson(customers[i]);
            }
            return jsonOk(s + "]");
        }
        if (action == "advance") {
            string id = param(P, "order_id");
            Order* o = findOrder(atoi(id.substr(id.size() > 3 ? 3 : 0).c_str()));
            if (!o) return jsonErr(400, "Order not found");
            advanceStatus(o);
            return jsonOk(orderJson(*o));
        }
        if (action == "process-next") {
            queue<int> q = pendingQueue();
            if (q.empty()) return jsonErr(400, "No pending orders in the queue");
            Order* o = findOrder(q.front());       // oldest order = front of queue
            q.pop();
            advanceStatus(o);
            return jsonOk(orderJson(*o));
        }
        if (action == "menu/add") {
            int rid = atoi(param(P, "restaurant_id").c_str());
            string name = cleanText(param(P, "name"));
            int price = atoi(param(P, "price").c_str());
            if (!findRestaurant(rid)) return jsonErr(400, "Restaurant not found");
            if (name.empty() || price <= 0) return jsonErr(400, "Enter a valid name and price");
            FoodItem m;
            m.id = 1;
            for (size_t i = 0; i < menu.size(); i++) m.id = max(m.id, menu[i].id + 1);
            m.rid = rid; m.name = name; m.price = price; m.veg = (param(P, "veg") != "0");
            menu.push_back(m);
            saveMenu();
            return jsonOk("{\"ok\":true}");
        }
        if (action == "menu/delete") {
            int id = atoi(param(P, "id").c_str());
            for (size_t i = 0; i < menu.size(); i++)
                if (menu[i].id == id) { menu.erase(menu.begin() + i); break; }
            saveMenu();
            return jsonOk("{\"ok\":true}");
        }
    }
    return jsonErr(404, "Not found");
}

// =====================================================================
//  TINY WEB SERVER
// =====================================================================
string urlDecode(const string& s) {
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < s.size() + 0 && isxdigit((unsigned char)s[i + 1]) && isxdigit((unsigned char)s[i + 2])) {
            out += (char)strtol(s.substr(i + 1, 2).c_str(), NULL, 16);
            i += 2;
        } else out += s[i];
    }
    return out;
}

void parseParams(const string& text, map<string, string>& P) {
    vector<string> pairs = split(text, '&');
    for (size_t i = 0; i < pairs.size(); i++) {
        size_t eq = pairs[i].find('=');
        if (eq == string::npos) continue;
        P[urlDecode(pairs[i].substr(0, eq))] = urlDecode(pairs[i].substr(eq + 1));
    }
}

string lower(string s) { for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]); return s; }

void serveClient(sock_t client) {
    // read the request (headers first, then the body)
    string data;
    char buf[4096];
    size_t headerEnd = string::npos;
    while (headerEnd == string::npos) {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) { CLOSESOCK(client); return; }     // empty / timed-out connection
        data.append(buf, n);
        headerEnd = data.find("\r\n\r\n");
        if (data.size() > 1000000) { CLOSESOCK(client); return; }
    }
    string head = data.substr(0, headerEnd);
    string lowHead = lower(head);

    size_t contentLength = 0;
    size_t cl = lowHead.find("content-length:");
    if (cl != string::npos) contentLength = (size_t)atoi(head.c_str() + cl + 15);
    while (data.size() < headerEnd + 4 + contentLength) {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) break;
        data.append(buf, n);
    }
    string body = data.substr(headerEnd + 4);

    string pin;
    size_t pp = lowHead.find("x-admin-pin:");
    if (pp != string::npos) {
        size_t e = head.find("\r\n", pp);
        pin = head.substr(pp + 12, e == string::npos ? string::npos : e - pp - 12);
        size_t a = pin.find_first_not_of(" \t");
        pin = (a == string::npos) ? "" : pin.substr(a);
    }

    // first line: METHOD /path?query HTTP/1.1
    istringstream firstLine(head);
    string method, target, version;
    firstLine >> method >> target >> version;
    string path = target, query;
    size_t q = target.find('?');
    if (q != string::npos) { path = target.substr(0, q); query = target.substr(q + 1); }

    map<string, string> P;
    parseParams(query, P);
    parseParams(body, P);

    Response r = handle(path, P, pin);

    const char* statusText = r.code == 200 ? "OK" : r.code == 400 ? "Bad Request" : r.code == 403 ? "Forbidden" : "Not Found";
    ostringstream out;
    out << "HTTP/1.1 " << r.code << " " << statusText << "\r\n"
        << "Content-Type: " << r.type << "\r\n"
        << "Content-Length: " << r.body.size() << "\r\n"
        << "Cache-Control: no-store\r\n"
        << "Connection: close\r\n\r\n" << r.body;
    string reply = out.str();
    size_t sent = 0;
    while (sent < reply.size()) {
        int n = send(client, reply.data() + sent, (int)(reply.size() - sent), 0);
        if (n <= 0) break;
        sent += n;
    }
    CLOSESOCK(client);
}

int main() {
    createStartingFiles();
    loadAll();

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { cout << "Could not start Winsock\n"; return 1; }
#endif

    sock_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) { cout << "Could not create socket\n"; return 1; }
    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");     // only this computer can connect
    addr.sin_port = htons(PORT);
    if (bind(server, (sockaddr*)&addr, sizeof(addr)) != 0) {
        cout << "Port " << PORT << " is busy. Close the other copy of this program and try again.\n";
        return 1;
    }
    listen(server, 16);

    cout << "==============================================\n";
    cout << " Path2Plate is running!\n";
    cout << " Open this in your browser: http://localhost:" << PORT << "\n";
    cout << " Admin PIN: " << ADMIN_PIN << "\n";
    cout << " (Press Ctrl+C to stop)\n";
    cout << "==============================================\n";
#ifdef _WIN32
    system("start http://localhost:8000");
#endif

    while (true) {
        sock_t client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) continue;
        // browsers sometimes open idle connections - don't wait for them forever
#ifdef _WIN32
        DWORD timeout = 500;
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
        struct timeval tv; tv.tv_sec = 0; tv.tv_usec = 500000;
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
        serveClient(client);
    }
    return 0;
}
