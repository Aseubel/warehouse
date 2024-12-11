#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <unordered_map>
using namespace std;

// 商品结构体
struct Goods {
    string code;        // 商品代码
    string name;        // 商品名称
    string category;    // 商品类别
    double price;       // 商品价格
    int totalQuantity;  // 商品总量
    time_t expirationDate;  // 保质期时间（以时间戳表示）
    vector<pair<time_t, int>> inOutRecords;  // 入库时间、入库数量记录，也可用于出库记录（数量为负数表示出库）
    int minStock;  // 最低库存量
};

// 出入库结构体，用于记录每一次出入库操作详情
struct InOutRecord {
    string code;        // 商品代码
    time_t inOutTime;   // 出/入库时间
    int quantity;       // 出/入库数量
    string operatorName;  // 出/入库经办人
};

// 工具类
class Util {
public:
    // 将输入的时间字符串转换为时间戳（格式：yyyy-MM-dd HH:mm:ss）
    static time_t formatTime(string timeStr) {
        if (timeStr.empty() || timeStr.length() < 19) {
            return time_t(-1);
        }
        istringstream ss(timeStr);
        tm tm = {};
        ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (ss.fail()) {
            return time_t(-1);
        }
        else {
            return mktime(&tm);
        }
    }

    // 将时间戳转换为字符串（格式：yyyy-MM-dd HH:mm:ss）
    static string formatTime(time_t timeStamp) {
        if (timeStamp == time_t(-1)) {
            return "";
        }
        tm* ptm = localtime(&timeStamp);
        char buffer[64] = {};
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ptm);
        return buffer;
    }
};

class WarehouseManagement {
private:
    unordered_map<string, int> codeIndexMap;  // 商品代码与索引的映射表

public:
    vector<Goods> goodsList;  // 存储商品信息的列表

    // 从文件读取商品信息初始化仓库
    void loadGoodsFromFile(const string& fileName) {
        ifstream file(fileName);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                if (line.empty() || line[0]=='\n' || line[0]=='#') {
                    continue;
                }
                Goods good;
                stringstream ss(line);
                getline(ss, good.code, ',');
                getline(ss, good.name, ',');
                getline(ss, good.category, ',');
                ss >> good.price;
                ss.ignore();
                ss >> good.totalQuantity;
                ss.ignore();
                ss >> good.expirationDate;
                ss.ignore();
                int recordCount;
                ss >> recordCount;
                ss.ignore();
                vector<pair<time_t, int>> inOutRecords;
                for (int i = 0; i < recordCount; ++i) {
                    time_t inOutTimeStr;
                    int quantity;
                    ss >> inOutTimeStr;
                    ss.ignore();
                    ss >> quantity;
                    ss.ignore();
                    inOutRecords.push_back(make_pair(inOutTimeStr, quantity));
                }
                good.inOutRecords = inOutRecords;
                ss >> good.minStock;
                ss.ignore();
                goodsList.push_back(good);
            }
            file.close();
        }
    }

    // 将仓库商品信息保存到文件
    void saveGoodsToFile(const string& fileName) {
        ofstream file(fileName);
        if (file.is_open()) {
            for (const auto& good : goodsList) {
                // 按格式将商品信息写入文件，例如：
                file << good.code << "," << good.name << "," << good.category << "," 
                    << good.price << "," << good.totalQuantity << "," << good.expirationDate << ",";
                file << good.inOutRecords.size() << ",";
                for (const auto& record : good.inOutRecords) {
                    file << record.first << "," << record.second << ",";
                }
                file << good.minStock << endl;
            }
            file.close();
        }
    }

    // 根据商品代码查找商品在列表中的索引
    int findGoodsIndex(const string& code) {
        if (codeIndexMap.find(code) != codeIndexMap.end()) {
            return codeIndexMap[code];
        }
        return -1;
    }

    // 建立商品代码与索引的映射表
    void buildCodeIndexMap() {
        for (size_t i = 0; i < goodsList.size(); ++i) {
            codeIndexMap[goodsList[i].code] = i;
        }
    }

    // 构造函数，从文件中读取商品信息初始化仓库
    WarehouseManagement(const string& fileName) {
        loadGoodsFromFile(fileName);
        buildCodeIndexMap();
    }

    // 商品入库功能
    void goodsIn(const InOutRecord& record) {
        int index = findGoodsIndex(record.code);
        if (index != -1) {
            goodsList[index].totalQuantity += record.quantity;
            goodsList[index].inOutRecords.push_back(make_pair(record.inOutTime, record.quantity));
            if (goodsList[index].totalQuantity < goodsList[index].minStock) {
                cout << "警告：商品库存仍然低于最低库存阈值！" << endl;
            }
        }
        else {
            Goods newGood = { record.code, "", "", 0, record.quantity, 0, {make_pair(record.inOutTime, record.quantity)} };
            cout << "商品不存在！请补充详细信息以添加商品" << endl;
            cout << "请输入商品名称：";
            cin >> newGood.name;
            cout << "请输入商品类别：";
            cin >> newGood.category;
            cout << "请输入商品价格: ";
            cin >> newGood.price;
            cout << "请输入商品最低库存阈值：";
            cin >> newGood.minStock;
            cout << "请输入保质期（格式：xxxx-xx-xx xx:xx:xx）: ";
            string timeStr;
            getline(cin, timeStr);
            getline(cin, timeStr);
            time_t expirationDate = Util::formatTime(timeStr);
            while (expirationDate == time_t(-1)) {
                cout << "时间格式错误，请重新输入: ";
                getline(cin, timeStr);
                expirationDate = Util::formatTime(timeStr);
            }
            newGood.expirationDate = expirationDate;
            goodsList.push_back(newGood);
        }
    }

    // 商品出库功能
    void goodsOut(const InOutRecord& record) {
        int index = findGoodsIndex(record.code);
        if (index != -1) {
            if (goodsList[index].totalQuantity >= record.quantity) {
                goodsList[index].totalQuantity -= record.quantity;
                goodsList[index].inOutRecords.push_back(make_pair(record.inOutTime, -record.quantity));  // 用负数表示出库数量
            }
            else {
                cout << "库存不足，出库失败！" << endl;
            }
        }
        else {
            cout << "商品不存在，出库失败！" << endl;
        }
    }

    // 显示所有商品信息
    void displayAllGoods() {
        for (const auto& good : goodsList) {
            cout << "商品代码: " << good.code << endl;
            cout << "商品名称: " << good.name << endl;
            cout << "商品类别: " << good.category << endl;
            cout << "商品价格: " << good.price << endl;
            cout << "商品总量: " << good.totalQuantity << endl;
            cout << "保质期：" << Util::formatTime(good.expirationDate) << endl;
            cout << "-------------------------" << endl;
        }
    }

    // 显示已过期商品信息
    void displayExpiredGoods() {
        time_t now = time(nullptr);
        for (const auto& good : goodsList) {
            if (good.expirationDate < now) {
                cout << "过期商品：" << good.name << endl;
                // 可详细输出更多该商品信息
            }
        }
    }

    // 显示某一类别的商品信息
    void displayCategoryGoods(const string& category) {
        for (const auto& good : goodsList) {
            if (good.category == category) {
                cout << "商品代码: " << good.code << endl;
                cout << "商品名称: " << good.name << endl;
                cout << "商品价格: " << good.price << endl;
                cout << "商品总量: " << good.totalQuantity << endl;
                cout << "-------------------------" << endl;
            }
        }
    }

    // 根据商品代码或名称查询商品所有信息
    void queryGoods(const string& query) {
        for (const auto& good : goodsList) {
            if (good.code == query || good.name == query) {
                cout << "商品代码: " << good.code << endl;
                cout << "商品名称: " << good.name << endl;
                cout << "商品类别: " << good.category << endl;
                cout << "商品价格: " << good.price << endl;
                cout << "商品总量: " << good.totalQuantity << endl;
                cout << "最低库存阈值：" << good.minStock << endl;
                cout << "保质期：" << Util::formatTime(good.expirationDate) << endl;
                cout << "出入库记录：" << endl;
                for (const auto& record : good.inOutRecords) {
                    cout << "    时间: " << Util::formatTime(record.first) << " 数量: " << record.second << endl;
                }
            }
        }
    }

    // 根据商品代码查询某个时间段的出入库信息
    void queryInOutRecords(const string& code, const string& startDate = "", const string& endDate = "") {
        int index = findGoodsIndex(code);
        time_t start = Util::formatTime(startDate);
        time_t end = Util::formatTime(endDate);
        if (index != -1) {
            for (const auto& record : goodsList[index].inOutRecords) {
                if ((start == time_t(-1) || record.first >= start) && (end == time_t(-1) || record.first <= end)) {
                    cout << "时间: " << Util::formatTime(record.first);
                    cout << "数量: " << record.second << endl;
                }
            }
        }
    }

    // 按入库日期查询入库信息
    void queryInByDate(const string& date, const string& endDate = "") {
        for (const auto& good : goodsList) {
            for (const auto& record : good.inOutRecords) {
                if (record.first == Util::formatTime(date) && record.second > 0) {
                    cout << "商品名称: " << good.name << " 入库数量: " << record.second << endl;
                }
            }
        }
        cout << "-------------------------" << endl;
    }

    // 按出库日期查询出库信息
    void queryOutByDate(const string& date, const string& endDate = "") {
        for (const auto& good : goodsList) {
            for (const auto& record : good.inOutRecords) {
                if (record.first == Util::formatTime(date) && record.second < 0) {
                    cout << "商品名称: " << good.name << " 入库数量: " << record.second << endl;
                }
            }
        }
        cout << "-------------------------" << endl;
    }

    // 显示快到保质期的商品（m天后过期）
    void displayAlmostExpiredGoods(int m) {
        time_t now = time(nullptr);
        time_t future = now + m * 24 * 60 * 60;  // m天后的时间戳
        for (const auto& good : goodsList) {
            if (good.expirationDate <= future && good.expirationDate > now) {
                cout << "快过期商品：" << good.name << endl;
            }
        }
    }

    // 按照类别排序商品
    void sortByCategory() {
        sort(goodsList.begin(), goodsList.end(), [](const Goods& a, const Goods& b) {
            return a.category < b.category;
            });
    }

    // 按照价格排序商品
    void sortByPrice() {
        sort(goodsList.begin(), goodsList.end(), [](const Goods& a, const Goods& b) {
            return a.price < b.price;
            });
    }

    // 按照保质期排序商品
    void sortByExpirationDate() {
        sort(goodsList.begin(), goodsList.end(), [](const Goods& a, const Goods& b) {
            return a.expirationDate < b.expirationDate;
            });
    }

    // 查询库存量低于阈值的商品
    void displayLowStockGoods() {
        for (const auto& good : goodsList) {
            if (good.totalQuantity < good.minStock) {
                cout << "商品代码: " << good.code << endl;
                cout << "商品名称: " << good.name << endl;
                cout << "商品类别: " << good.category << endl;
                cout << "商品价格: " << good.price << endl;
                cout << "商品总量: " << good.totalQuantity << endl;
                cout << "保质期：" << Util::formatTime(good.expirationDate) << endl;
                cout << "最低库存阈值：" << good.minStock << endl;
                cout << "-------------------------" << endl;
            }
        }
    }

    // 设置最低库存阈值的函数
    void setMinStock(const string& code, int minStock) {
        int index = findGoodsIndex(code);
        if (index != -1) {
            goodsList[index].minStock = minStock;
        }
    }
};

int main() {
    WarehouseManagement warehouse("goods.txt");  // 从goods.txt文件初始化仓库

    int choice;
    do {
        cout << "0. 清屏" << endl;
        cout << "1. 商品入库" << endl;
        cout << "2. 商品出库" << endl;
        cout << "3. 显示所有商品信息" << endl;
        cout << "4. 显示已过期商品信息" << endl;
        cout << "5. 显示某一类别的商品信息" << endl;
        cout << "6. 查询商品信息" << endl;
        cout << "7. 查询商品出入库信息" << endl;
        cout << "8. 查询入库信息（按日期）" << endl;
        cout << "9. 查询出库信息（按日期）" << endl;
        cout << "10. 显示快到保质期商品" << endl;
        cout << "11. 排序功能" << endl;
        cout << "12. 查询所有库存量低于阈值的商品" << endl;
        cout << "13. 设置商品最低库存阈值" << endl;
        cout << "14. 退出" << endl;
        cout << "请输入你的选择: ";
        cin >> choice;

        switch (choice) {
        case 0: {
            system("cls");
            break;
        }
        case 1: {
            InOutRecord record;
            cout << "请输入商品代码: ";
            cin >> record.code;
            cout << "请输入入库时间（格式：xxxx-xx-xx xx:xx:xx）: ";
            string timeStr;
            getline(cin, timeStr);
            getline(cin, timeStr);
            time_t inTime = Util::formatTime(timeStr);
            while (inTime == time_t(-1)) {
                cout << "时间格式错误，请重新输入: ";
                getline(cin, timeStr);
                inTime = Util::formatTime(timeStr);
            }
            record.inOutTime = inTime;
            cout << "请输入入库数量: ";
            cin >> record.quantity;
            cout << "请输入经办人: ";
            cin >> record.operatorName;
            warehouse.goodsIn(record);
            break;
        }
        case 2: {
            InOutRecord record;
            cout << "请输入商品代码: ";
            cin >> record.code;
            // 类似入库操作，输入出库相关信息，省略重复代码
            warehouse.goodsOut(record);
            break;
        }
        case 3:
            warehouse.displayAllGoods();
            break;
        case 4:
            warehouse.displayExpiredGoods();
            break;
        case 5: {
            string category;
            cout << "请输入商品类别: ";
            cin >> category;
            warehouse.displayCategoryGoods(category);
            break;
        }
        case 6: {
            string query;
            cout << "请输入商品代码或名称: ";
            cin >> query;
            warehouse.queryGoods(query);
            break;
        }
        case 7: {
            string code;
            cout << "请输入商品代码: ";
            cin >> code;
            string start, end;
            cout << "请输入开始时间（可选，格式：xxxx-xx-xx xx:xx:xx，直接回车表示无开始时间限制）: ";
            getline(cin, start);  // 处理上一次输入的换行符遗留问题
            getline(cin, start);
            cout << "请输入结束时间（可选，格式：xxxx-xx-xx xx:xx:xx，直接回车表示无结束时间限制）: ";
            getline(cin, end);
            warehouse.queryInOutRecords(code, start, end);
            break;
        }
        case 8: {
            string start, end;
            cout << "请输入开始时间（格式：xxxx-xx-xx xx:xx:xx，直接回车表示当天）: ";
            getline(cin, start);
            cout << "请输入结束时间（直接回车表示与开始时间相同）: ";
            getline(cin, end);
            warehouse.queryInByDate(start, end);
            break;
        }
        case 9: {
            string start, end;
            cout << "请输入开始时间（格式：xxxx-xx-xx xx:xx:xx，直接回车表示当天）: ";
            getline(cin, start);
            cout << "请输入结束时间（直接回车表示与开始时间相同）: ";
            getline(cin, end);
            warehouse.queryOutByDate(start, end);
            break;
        }
        case 10: {
            int m;
            cout << "请输入还有多少天就过期的天数: ";
            cin >> m;
            warehouse.displayAlmostExpiredGoods(m);
            break;
        }
        case 11: {
            int sortChoice;
            cout << "1. 按类别排序" << endl;
            cout << "2. 按价格排序" << endl;
            cout << "3. 按保质期排序" << endl;
            cout << "请输入排序方式选择: ";
            cin >> sortChoice;
            switch (sortChoice) {
            case 1:
                warehouse.sortByCategory();
                break;
            case 2:
                warehouse.sortByPrice();
                break;
            case 3:
                warehouse.sortByExpirationDate();
                break;
            default:
                cout << "无效的排序选择！" << endl;
            }
            break;
        }
        case 12: {
            string code;
            cout << "请输入商品代码: ";
            cin >> code;
            int minStock;
            cout << "请输入最低库存阈值: ";
            cin >> minStock;
            warehouse.setMinStock(code, minStock);
            break;
        }
        case 13: {
            warehouse.setMinStock(code, minStock);
            break;
        }
        case 14: {
            warehouse.saveGoodsToFile("goods.txt");  // 退出前保存数据到文件
            cout << "感谢使用！" << endl;
            break;
        }
        default:
            cout << "无效的选择，请重新输入！" << endl;
        }

    } while (choice != 12);

    return 0;
}