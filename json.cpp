#include "json.h"

using namespace std;

namespace Json {

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::AsBool() const {
        return std::get<bool>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    int Node::AsInt() const {
        return std::get<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    double Node::AsDouble() const {
        return IsPureDouble() ? std::get<double>(*this) : AsInt();
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::monostate> (*this);
    }

    const Node& Document::GetRoot() const {
        return root;
    }

    Node LoadArray(istream& input) {
        vector<Node> result;
        char c;
        while ((input >> c) && (c != ']')) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadBool(istream& input) {
        string s;
        while (isalpha(input.peek())) {
            s.push_back(input.get());
        }
        return Node(s == "true");
    }

    Node LoadNull(istream& input) {
        string s;
        while (isalpha(input.peek())) {
            s.push_back(input.get());
        }
        return Node(0);
    }
    Node LoadNumber(istream& input) {
        bool isNegative = false;
        if (input.peek() == '-') {
            isNegative = true;
            input.get();
        }
        int intPart = 0;
        while (isdigit(input.peek())) {
            intPart *= 10;
            intPart += input.get() - '0';
        }
        if (input.peek() != '.') {
            return Node(intPart * (isNegative ? -1 : 1));
        }
        input.get();  // '.'
        double result = intPart;
        double fracMult = 0.1;
        while (isdigit(input.peek())) {
            result += fracMult * (input.get() - '0');
            fracMult /= 10;
        }
        return Node(result * (isNegative ? -1 : 1));
    }

    Node LoadString(istream& input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node LoadDict(istream& input) {
        Dict result;
        char c;
        while ((input >> c) && (c != '}')) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.emplace(move(key), LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if (c == 't' || c == 'f') {
            input.putback(c);
            return LoadBool(input);
        } else if (c == 'n') {
            input.putback(c);
            return LoadNull(input);
        } else {
            input.putback(c);
            return LoadNumber(input);
        }
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    template<>
    void PrintValue<string>(const string& value, ostream& output) {
        output << '"';
        for (const char c : value) {
            if (c == '"' || c == '\\') {
                output << '\\';
            }
            output << c;
        }
        output << '"';
    }

    template<>
    void PrintValue<bool>(const bool& value, std::ostream& output) {
        output << std::boolalpha << value <<std::noboolalpha;
    }

    template<>
    void PrintValue<Array>(const Array& nodes, std::ostream& output) {
        output << '[';
        bool first = true;
        for (const Node& node : nodes) {
            if (!first) {
                output << ", ";
            }
            first = false;
            PrintNode(node, output);
        }
        output << ']';
    }

    template<>
    void PrintValue<Dict>(const Dict& dict, std::ostream& output) {
        output << '{';
        bool first = true;
        for (const auto&[key, node]: dict) {
            if (!first) {
                output << ", ";
            }
            first = false;
            PrintValue(key, output);
            output << ": ";
            PrintNode(node, output);
        }
        output << '}';
    }

    template<>
    void PrintValue<std::monostate>(const std::monostate&, std::ostream& output) {
        output << "null";
    }

    void PrintNode(const Json::Node& node, ostream& output) {
        visit([&output](const auto& value) { PrintValue(value, output); },
              node.GetBase());
    }

    void Print(const Document& document, ostream& output) {
        PrintNode(document.GetRoot(), output);
    }


}
