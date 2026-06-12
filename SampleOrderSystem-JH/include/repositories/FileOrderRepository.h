#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "repositories/IOrderRepository.h"

namespace repositories {

// Not thread-safe. Intended for single-threaded console use only.
// TODO: Move implementation to FileOrderRepository.cpp when project transitions to production build.
class FileOrderRepository : public IOrderRepository {
public:
    explicit FileOrderRepository(const std::string& file_path);

    [[nodiscard]] int                          Save(const models::Order& order) override;
    [[nodiscard]] std::optional<models::Order> FindById(int id) const noexcept override;
    [[nodiscard]] std::vector<models::Order>   FindAll() const noexcept override;
    [[nodiscard]] std::vector<models::Order>   FindByStatus(models::OrderStatus status) const noexcept override;
    [[nodiscard]] std::vector<models::Order>   FindByProductId(int product_id) const noexcept override;
    [[nodiscard]] bool                         Update(const models::Order& order) noexcept override;

private:
    std::string file_path_;
    int         next_id_ = 1;

    void                       SaveToFile(const std::vector<models::Order>& orders) const;
    std::vector<models::Order> LoadFromFile() const;
    static std::string         OrderToJson(const models::Order& order);
    static models::Order       JsonToOrder(const std::string& json_line);
    static std::string         StatusToString(models::OrderStatus status);
    static models::OrderStatus StringToStatus(const std::string& s);
};

// --- implementations ---

inline std::string FileOrderRepository::StatusToString(models::OrderStatus status) {
    switch (status) {
        case models::OrderStatus::RESERVED:  return "RESERVED";
        case models::OrderStatus::CONFIRMED: return "CONFIRMED";
        case models::OrderStatus::PRODUCING: return "PRODUCING";
        case models::OrderStatus::RELEASE:   return "RELEASE";
        case models::OrderStatus::REJECTED:  return "REJECTED";
        default:                             return "RESERVED";
    }
}

inline models::OrderStatus FileOrderRepository::StringToStatus(const std::string& s) {
    if (s == "CONFIRMED") return models::OrderStatus::CONFIRMED;
    if (s == "PRODUCING") return models::OrderStatus::PRODUCING;
    if (s == "RELEASE")   return models::OrderStatus::RELEASE;
    if (s == "REJECTED")  return models::OrderStatus::REJECTED;
    return models::OrderStatus::RESERVED;
}

inline std::string FileOrderRepository::OrderToJson(const models::Order& o) {
    std::ostringstream ss;
    ss << "{\"id\":" << o.id
       << ",\"product_id\":" << o.product_id
       << ",\"quantity\":" << o.quantity
       << ",\"deadline\":\"" << o.deadline << "\""
       << ",\"status\":\"" << StatusToString(o.status) << "\""
       << ",\"required_batches\":" << o.required_batches
       << ",\"estimated_yield\":" << o.estimated_yield
       << ",\"created_at\":\"" << o.created_at << "\""
       << "}";
    return ss.str();
}

inline models::Order FileOrderRepository::JsonToOrder(const std::string& line) {
    auto extractStr = [&](const std::string& key) -> std::string {
        auto pos = line.find("\"" + key + "\":\"");
        if (pos == std::string::npos) return "";
        pos += key.size() + 4;
        auto end = line.find("\"", pos);
        return line.substr(pos, end - pos);
    };
    auto extractNum = [&](const std::string& key) -> std::string {
        auto pos = line.find("\"" + key + "\":");
        if (pos == std::string::npos) return "0";
        pos += key.size() + 3;
        auto end = line.find_first_of(",}", pos);
        return line.substr(pos, end - pos);
    };

    models::Order o;
    o.id               = std::stoi(extractNum("id"));
    o.product_id       = std::stoi(extractNum("product_id"));
    o.quantity         = std::stoi(extractNum("quantity"));
    o.deadline         = extractStr("deadline");
    o.status           = StringToStatus(extractStr("status"));
    o.required_batches = std::stoi(extractNum("required_batches"));
    o.estimated_yield  = std::stod(extractNum("estimated_yield"));
    o.created_at       = extractStr("created_at");
    return o;
}

inline void FileOrderRepository::SaveToFile(const std::vector<models::Order>& orders) const {
    std::ofstream ofs(file_path_, std::ios::trunc);
    for (const auto& o : orders) {
        ofs << OrderToJson(o) << "\n";
    }
}

inline std::vector<models::Order> FileOrderRepository::LoadFromFile() const {
    std::vector<models::Order> result;
    std::ifstream ifs(file_path_);
    if (!ifs.is_open()) return result;
    std::string line;
    while (std::getline(ifs, line)) {
        if (!line.empty()) result.push_back(JsonToOrder(line));
    }
    return result;
}

inline FileOrderRepository::FileOrderRepository(const std::string& file_path)
    : file_path_(file_path) {
    auto existing = LoadFromFile();
    for (const auto& o : existing) {
        if (o.id >= next_id_) next_id_ = o.id + 1;
    }
}

inline int FileOrderRepository::Save(const models::Order& order) {
    auto orders = LoadFromFile();
    models::Order stored = order;
    stored.id = next_id_++;
    orders.push_back(stored);
    SaveToFile(orders);
    return stored.id;
}

inline std::optional<models::Order> FileOrderRepository::FindById(int id) const noexcept {
    for (const auto& o : LoadFromFile())
        if (o.id == id) return o;
    return std::nullopt;
}

inline std::vector<models::Order> FileOrderRepository::FindAll() const noexcept {
    return LoadFromFile();
}

inline std::vector<models::Order> FileOrderRepository::FindByStatus(models::OrderStatus status) const noexcept {
    std::vector<models::Order> result;
    for (const auto& o : LoadFromFile())
        if (o.status == status) result.push_back(o);
    return result;
}

inline std::vector<models::Order> FileOrderRepository::FindByProductId(int product_id) const noexcept {
    std::vector<models::Order> result;
    for (const auto& o : LoadFromFile())
        if (o.product_id == product_id) result.push_back(o);
    return result;
}

inline bool FileOrderRepository::Update(const models::Order& order) noexcept {
    auto orders = LoadFromFile();
    for (auto& o : orders) {
        if (o.id == order.id) {
            o = order;
            SaveToFile(orders);
            return true;
        }
    }
    return false;
}

}  // namespace repositories
