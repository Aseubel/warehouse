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

// ��Ʒ�ṹ��
struct Goods {
    string code;        // ��Ʒ����
    string name;        // ��Ʒ����
    string category;    // ��Ʒ���
    double price;       // ��Ʒ�۸�
    int totalQuantity;  // ��Ʒ����
    time_t expirationDate;  // ������ʱ�䣨��ʱ�����ʾ��
    vector<pair<time_t, int>> inOutRecords;  // ���ʱ�䡢���������¼��Ҳ�����ڳ����¼������Ϊ������ʾ���⣩
    int minStock;  // ��Ϳ����
};

// �����ṹ�壬���ڼ�¼ÿһ�γ�����������
struct InOutRecord {
    string code;        // ��Ʒ����
    time_t inOutTime;   // ��/���ʱ��
    int quantity;       // ��/�������
    string operatorName;  // ��/��⾭����
};

// ������
class Util {
public:
    // �������ʱ���ַ���ת��Ϊʱ�������ʽ��yyyy-MM-dd HH:mm:ss��
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

    // ��ʱ���ת��Ϊ�ַ�������ʽ��yyyy-MM-dd HH:mm:ss��
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
    unordered_map<string, int> codeIndexMap;  // ��Ʒ������������ӳ���

public:
    vector<Goods> goodsList;  // �洢��Ʒ��Ϣ���б�

    // ���ļ���ȡ��Ʒ��Ϣ��ʼ���ֿ�
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

    // ���ֿ���Ʒ��Ϣ���浽�ļ�
    void saveGoodsToFile(const string& fileName) {
        ofstream file(fileName);
        if (file.is_open()) {
            for (const auto& good : goodsList) {
                // ����ʽ����Ʒ��Ϣд���ļ������磺
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

    // ������Ʒ���������Ʒ���б��е�����
    int findGoodsIndex(const string& code) {
        if (codeIndexMap.find(code) != codeIndexMap.end()) {
            return codeIndexMap[code];
        }
        return -1;
    }

    // ������Ʒ������������ӳ���
    void buildCodeIndexMap() {
        for (size_t i = 0; i < goodsList.size(); ++i) {
            codeIndexMap[goodsList[i].code] = i;
        }
    }

    // ���캯�������ļ��ж�ȡ��Ʒ��Ϣ��ʼ���ֿ�
    WarehouseManagement(const string& fileName) {
        loadGoodsFromFile(fileName);
        buildCodeIndexMap();
    }

    // ��Ʒ��⹦��
    void goodsIn(const InOutRecord& record) {
        int index = findGoodsIndex(record.code);
        if (index != -1) {
            goodsList[index].totalQuantity += record.quantity;
            goodsList[index].inOutRecords.push_back(make_pair(record.inOutTime, record.quantity));
            if (goodsList[index].totalQuantity < goodsList[index].minStock) {
                cout << "���棺��Ʒ�����Ȼ������Ϳ����ֵ��" << endl;
            }
        }
        else {
            Goods newGood = { record.code, "", "", 0, record.quantity, 0, {make_pair(record.inOutTime, record.quantity)} };
            cout << "��Ʒ�����ڣ��벹����ϸ��Ϣ�������Ʒ" << endl;
            cout << "��������Ʒ���ƣ�";
            cin >> newGood.name;
            cout << "��������Ʒ���";
            cin >> newGood.category;
            cout << "��������Ʒ�۸�: ";
            cin >> newGood.price;
            cout << "��������Ʒ��Ϳ����ֵ��";
            cin >> newGood.minStock;
            cout << "�����뱣���ڣ���ʽ��xxxx-xx-xx xx:xx:xx��: ";
            string timeStr;
            getline(cin, timeStr);
            getline(cin, timeStr);
            time_t expirationDate = Util::formatTime(timeStr);
            while (expirationDate == time_t(-1)) {
                cout << "ʱ���ʽ��������������: ";
                getline(cin, timeStr);
                expirationDate = Util::formatTime(timeStr);
            }
            newGood.expirationDate = expirationDate;
            goodsList.push_back(newGood);
        }
    }

    // ��Ʒ���⹦��
    void goodsOut(const InOutRecord& record) {
        int index = findGoodsIndex(record.code);
        if (index != -1) {
            if (goodsList[index].totalQuantity >= record.quantity) {
                goodsList[index].totalQuantity -= record.quantity;
                goodsList[index].inOutRecords.push_back(make_pair(record.inOutTime, -record.quantity));  // �ø�����ʾ��������
            }
            else {
                cout << "��治�㣬����ʧ�ܣ�" << endl;
            }
        }
        else {
            cout << "��Ʒ�����ڣ�����ʧ�ܣ�" << endl;
        }
    }

    // ��ʾ������Ʒ��Ϣ
    void displayAllGoods() {
        for (const auto& good : goodsList) {
            cout << "��Ʒ����: " << good.code << endl;
            cout << "��Ʒ����: " << good.name << endl;
            cout << "��Ʒ���: " << good.category << endl;
            cout << "��Ʒ�۸�: " << good.price << endl;
            cout << "��Ʒ����: " << good.totalQuantity << endl;
            cout << "�����ڣ�" << Util::formatTime(good.expirationDate) << endl;
            cout << "-------------------------" << endl;
        }
    }

    // ��ʾ�ѹ�����Ʒ��Ϣ
    void displayExpiredGoods() {
        time_t now = time(nullptr);
        for (const auto& good : goodsList) {
            if (good.expirationDate < now) {
                cout << "������Ʒ��" << good.name << endl;
                // ����ϸ����������Ʒ��Ϣ
            }
        }
    }

    // ��ʾĳһ������Ʒ��Ϣ
    void displayCategoryGoods(const string& category) {
        for (const auto& good : goodsList) {
            if (good.category == category) {
                cout << "��Ʒ����: " << good.code << endl;
                cout << "��Ʒ����: " << good.name << endl;
                cout << "��Ʒ�۸�: " << good.price << endl;
                cout << "��Ʒ����: " << good.totalQuantity << endl;
                cout << "-------------------------" << endl;
            }
        }
    }

    // ������Ʒ��������Ʋ�ѯ��Ʒ������Ϣ
    void queryGoods(const string& query) {
        for (const auto& good : goodsList) {
            if (good.code == query || good.name == query) {
                cout << "��Ʒ����: " << good.code << endl;
                cout << "��Ʒ����: " << good.name << endl;
                cout << "��Ʒ���: " << good.category << endl;
                cout << "��Ʒ�۸�: " << good.price << endl;
                cout << "��Ʒ����: " << good.totalQuantity << endl;
                cout << "��Ϳ����ֵ��" << good.minStock << endl;
                cout << "�����ڣ�" << Util::formatTime(good.expirationDate) << endl;
                cout << "������¼��" << endl;
                for (const auto& record : good.inOutRecords) {
                    cout << "    ʱ��: " << Util::formatTime(record.first) << " ����: " << record.second << endl;
                }
            }
        }
    }

    // ������Ʒ�����ѯĳ��ʱ��εĳ������Ϣ
    void queryInOutRecords(const string& code, const string& startDate = "", const string& endDate = "") {
        int index = findGoodsIndex(code);
        time_t start = Util::formatTime(startDate);
        time_t end = Util::formatTime(endDate);
        if (index != -1) {
            for (const auto& record : goodsList[index].inOutRecords) {
                if ((start == time_t(-1) || record.first >= start) && (end == time_t(-1) || record.first <= end)) {
                    cout << "ʱ��: " << Util::formatTime(record.first);
                    cout << "����: " << record.second << endl;
                }
            }
        }
    }

    // ��������ڲ�ѯ�����Ϣ
    void queryInByDate(const string& date, const string& endDate = "") {
        for (const auto& good : goodsList) {
            for (const auto& record : good.inOutRecords) {
                if (record.first == Util::formatTime(date) && record.second > 0) {
                    cout << "��Ʒ����: " << good.name << " �������: " << record.second << endl;
                }
            }
        }
        cout << "-------------------------" << endl;
    }

    // ���������ڲ�ѯ������Ϣ
    void queryOutByDate(const string& date, const string& endDate = "") {
        for (const auto& good : goodsList) {
            for (const auto& record : good.inOutRecords) {
                if (record.first == Util::formatTime(date) && record.second < 0) {
                    cout << "��Ʒ����: " << good.name << " �������: " << record.second << endl;
                }
            }
        }
        cout << "-------------------------" << endl;
    }

    // ��ʾ�쵽�����ڵ���Ʒ��m�����ڣ�
    void displayAlmostExpiredGoods(int m) {
        time_t now = time(nullptr);
        time_t future = now + m * 24 * 60 * 60;  // m����ʱ���
        for (const auto& good : goodsList) {
            if (good.expirationDate <= future && good.expirationDate > now) {
                cout << "�������Ʒ��" << good.name << endl;
            }
        }
    }

    // �������������Ʒ
    void sortByCategory() {
        sort(goodsList.begin(), goodsList.end(), [](const Goods& a, const Goods& b) {
            return a.category < b.category;
            });
    }

    // ���ռ۸�������Ʒ
    void sortByPrice() {
        sort(goodsList.begin(), goodsList.end(), [](const Goods& a, const Goods& b) {
            return a.price < b.price;
            });
    }

    // ���ձ�����������Ʒ
    void sortByExpirationDate() {
        sort(goodsList.begin(), goodsList.end(), [](const Goods& a, const Goods& b) {
            return a.expirationDate < b.expirationDate;
            });
    }

    // ��ѯ�����������ֵ����Ʒ
    void displayLowStockGoods() {
        for (const auto& good : goodsList) {
            if (good.totalQuantity < good.minStock) {
                cout << "��Ʒ����: " << good.code << endl;
                cout << "��Ʒ����: " << good.name << endl;
                cout << "��Ʒ���: " << good.category << endl;
                cout << "��Ʒ�۸�: " << good.price << endl;
                cout << "��Ʒ����: " << good.totalQuantity << endl;
                cout << "�����ڣ�" << Util::formatTime(good.expirationDate) << endl;
                cout << "��Ϳ����ֵ��" << good.minStock << endl;
                cout << "-------------------------" << endl;
            }
        }
    }

    // ������Ϳ����ֵ�ĺ���
    void setMinStock(const string& code, int minStock) {
        int index = findGoodsIndex(code);
        if (index != -1) {
            goodsList[index].minStock = minStock;
        }
    }
};

int main() {
    WarehouseManagement warehouse("goods.txt");  // ��goods.txt�ļ���ʼ���ֿ�

    int choice;
    do {
        cout << "0. ����" << endl;
        cout << "1. ��Ʒ���" << endl;
        cout << "2. ��Ʒ����" << endl;
        cout << "3. ��ʾ������Ʒ��Ϣ" << endl;
        cout << "4. ��ʾ�ѹ�����Ʒ��Ϣ" << endl;
        cout << "5. ��ʾĳһ������Ʒ��Ϣ" << endl;
        cout << "6. ��ѯ��Ʒ��Ϣ" << endl;
        cout << "7. ��ѯ��Ʒ�������Ϣ" << endl;
        cout << "8. ��ѯ�����Ϣ�������ڣ�" << endl;
        cout << "9. ��ѯ������Ϣ�������ڣ�" << endl;
        cout << "10. ��ʾ�쵽��������Ʒ" << endl;
        cout << "11. ������" << endl;
        cout << "12. ��ѯ���п����������ֵ����Ʒ" << endl;
        cout << "13. ������Ʒ��Ϳ����ֵ" << endl;
        cout << "14. �˳�" << endl;
        cout << "���������ѡ��: ";
        cin >> choice;

        switch (choice) {
        case 0: {
            system("cls");
            break;
        }
        case 1: {
            InOutRecord record;
            cout << "��������Ʒ����: ";
            cin >> record.code;
            cout << "���������ʱ�䣨��ʽ��xxxx-xx-xx xx:xx:xx��: ";
            string timeStr;
            getline(cin, timeStr);
            getline(cin, timeStr);
            time_t inTime = Util::formatTime(timeStr);
            while (inTime == time_t(-1)) {
                cout << "ʱ���ʽ��������������: ";
                getline(cin, timeStr);
                inTime = Util::formatTime(timeStr);
            }
            record.inOutTime = inTime;
            cout << "�������������: ";
            cin >> record.quantity;
            cout << "�����뾭����: ";
            cin >> record.operatorName;
            warehouse.goodsIn(record);
            break;
        }
        case 2: {
            InOutRecord record;
            cout << "��������Ʒ����: ";
            cin >> record.code;
            // ������������������������Ϣ��ʡ���ظ�����
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
            cout << "��������Ʒ���: ";
            cin >> category;
            warehouse.displayCategoryGoods(category);
            break;
        }
        case 6: {
            string query;
            cout << "��������Ʒ���������: ";
            cin >> query;
            warehouse.queryGoods(query);
            break;
        }
        case 7: {
            string code;
            cout << "��������Ʒ����: ";
            cin >> code;
            string start, end;
            cout << "�����뿪ʼʱ�䣨��ѡ����ʽ��xxxx-xx-xx xx:xx:xx��ֱ�ӻس���ʾ�޿�ʼʱ�����ƣ�: ";
            getline(cin, start);  // ������һ������Ļ��з���������
            getline(cin, start);
            cout << "���������ʱ�䣨��ѡ����ʽ��xxxx-xx-xx xx:xx:xx��ֱ�ӻس���ʾ�޽���ʱ�����ƣ�: ";
            getline(cin, end);
            warehouse.queryInOutRecords(code, start, end);
            break;
        }
        case 8: {
            string start, end;
            cout << "�����뿪ʼʱ�䣨��ʽ��xxxx-xx-xx xx:xx:xx��ֱ�ӻس���ʾ���죩: ";
            getline(cin, start);
            cout << "���������ʱ�䣨ֱ�ӻس���ʾ�뿪ʼʱ����ͬ��: ";
            getline(cin, end);
            warehouse.queryInByDate(start, end);
            break;
        }
        case 9: {
            string start, end;
            cout << "�����뿪ʼʱ�䣨��ʽ��xxxx-xx-xx xx:xx:xx��ֱ�ӻس���ʾ���죩: ";
            getline(cin, start);
            cout << "���������ʱ�䣨ֱ�ӻس���ʾ�뿪ʼʱ����ͬ��: ";
            getline(cin, end);
            warehouse.queryOutByDate(start, end);
            break;
        }
        case 10: {
            int m;
            cout << "�����뻹�ж�����͹��ڵ�����: ";
            cin >> m;
            warehouse.displayAlmostExpiredGoods(m);
            break;
        }
        case 11: {
            int sortChoice;
            cout << "1. ���������" << endl;
            cout << "2. ���۸�����" << endl;
            cout << "3. ������������" << endl;
            cout << "����������ʽѡ��: ";
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
                cout << "��Ч������ѡ��" << endl;
            }
            break;
        }
        case 12: {
            string code;
            cout << "��������Ʒ����: ";
            cin >> code;
            int minStock;
            cout << "��������Ϳ����ֵ: ";
            cin >> minStock;
            warehouse.setMinStock(code, minStock);
            break;
        }
        case 13: {
            warehouse.setMinStock(code, minStock);
            break;
        }
        case 14: {
            warehouse.saveGoodsToFile("goods.txt");  // �˳�ǰ�������ݵ��ļ�
            cout << "��лʹ�ã�" << endl;
            break;
        }
        default:
            cout << "��Ч��ѡ�����������룡" << endl;
        }

    } while (choice != 12);

    return 0;
}