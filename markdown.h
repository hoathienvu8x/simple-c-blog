#ifndef _MARKDOWN_H
#define _MARKDOWN_H

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <cctype>
#include <regex>
#include <algorithm>

class BlockParser {
    public:
        BlockParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : result("", std::ios_base::ate | std::ios_base::in | std::ios_base::out)
        , childParser(nullptr)
        , parseLineCallback(parseLineCallback)
        , getBlockParserForLineCallback(getBlockParserForLineCallback)
        {}
        virtual ~BlockParser() {}
        virtual void AddLine(std::string& line) {
            this->parseBlock(line);
            if (this->isInlineBlockAllowed() && !this->childParser) {
                this->childParser = this->getBlockParserForLine(line);
            }
            if (this->childParser) {
                this->childParser->AddLine(line);
                if (this->childParser->IsFinished()) {
                    this->result << this->childParser->GetResult().str();
                    this->childParser = nullptr;
                }
                return;
            }
            if (this->isLineParserAllowed()) {
                this->parseLine(line);
            }
            this->result << line;
        }
        virtual bool IsFinished() const = 0;
        std::stringstream& GetResult() { return this->result; }
        void Clear() { this->result.str("");  }
    protected:
        std::stringstream result;
        std::shared_ptr<BlockParser> childParser;
        virtual bool isInlineBlockAllowed() const = 0;
        virtual bool isLineParserAllowed() const = 0;
        virtual void parseBlock(std::string& line) = 0;
        void parseLine(std::string& line) {
            if (parseLineCallback) {
                parseLineCallback(line);
            }
        }
        uint32_t getIndentationWidth(const std::string& line) const {
            bool hasMetNonSpace = false;
            uint32_t indentation = static_cast<uint32_t>(
                std::count_if(
                    line.begin(),
                    line.end(),
                    [&hasMetNonSpace](unsigned char c) {
                        if (hasMetNonSpace) {
                            return false;
                        }
                        if (std::isspace(c)) {
                            return true;
                        }
                        hasMetNonSpace = true;
                        return false;
                    }
                )
            );
            return indentation;
        }
        std::shared_ptr<BlockParser> getBlockParserForLine(const std::string& line) {
            if (getBlockParserForLineCallback) {
                return getBlockParserForLineCallback(line);
            }
            return nullptr;
        }
    private:
        std::function<void(std::string&)> parseLineCallback;
        std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback;
}; // class BlockParser

class ChecklistParser : public BlockParser {
    public:
        ChecklistParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^- \\[[x| ]\\] .*");
            return std::regex_match(line, re);
        }
        bool IsFinished() const override { return this->isFinished; }
    protected:
        bool isInlineBlockAllowed() const override { return true; }
        bool isLineParserAllowed() const override { return true; }
        void parseBlock(std::string& line) override {
            bool isStartOfNewListItem = IsStartingLine(line);
            uint32_t indentation = getIndentationWidth(line);
            static std::regex lineRegex("^(- )");
            line = std::regex_replace(line, lineRegex, "");
            static std::regex emptyBoxRegex("^\\[ \\]");
            static std::string emptyBoxReplacement = "<input type=\"checkbox\"/>";
            line = std::regex_replace(line, emptyBoxRegex, emptyBoxReplacement);
            static std::regex boxRegex("^\\[x\\]");
            static std::string boxReplacement = "<input type=\"checkbox\" checked=\"checked\"/>";
            line = std::regex_replace(line, boxRegex, boxReplacement);
            if (!this->isStarted) {
                line = "<ul class=\"checklist\"><li><label>" + line;
                this->isStarted = true;
                return;
            }
            if (indentation >= 2) {
                line = line.substr(2);
                return;
            }
            if (
                line.empty() ||
                line.find("</label></li><li><label>") != std::string::npos ||
                line.find("</label></li></ul>") != std::string::npos
            ) {
                line = "</label></li></ul>" + line;
                this->isFinished = true;
                return;
            }
            if (isStartOfNewListItem) {
                line = "</label></li><li><label>" + line;
            }
        }
    private:
        bool isStarted;
        bool isFinished;
}; // class ChecklistParser
class CodeBlockParser : public BlockParser {
    public:
        CodeBlockParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^(?:`){3}$");
            return std::regex_match(line, re);
        }
        bool IsFinished() const override { return this->isFinished; }
    protected:
        bool isInlineBlockAllowed() const override { return false; }
        bool isLineParserAllowed() const override { return false; }
        void parseBlock(std::string& line) override {
            if (line == "```") {
                if (!this->isStarted) {
                    line = "<pre><code>\n";
                    this->isStarted = true;
                    this->isFinished = false;
                    return;
                } else {
                    line = "</code></pre>";
                    this->isFinished = true;
                    this->isStarted = false;
                    return;
                }
            }
            line += "\n";
        }
    private:
        bool isStarted;
        bool isFinished;
}; // class CodeBlockParser
class EmphasizedParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            static std::regex re("(?!.*`.*|.*<code>.*)_(?!.*`.*|.*<\\/code>.*)([^_]*)_(?!.*`.*|.*<\\/code>.*)");
            static std::string replacement = "<em>$1</em>";
            line = std::regex_replace(line, re, replacement);
        }
}; // class EmphasizedParser
class HeadlineParser : public BlockParser {
    public:
        HeadlineParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        ) : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^(?:#){1,6} (.*)");
            return std::regex_match(line, re);
        }
        bool IsFinished() const override { return true; }
    protected:
        bool isInlineBlockAllowed() const override { return false; }
        bool isLineParserAllowed() const override { return false; }
    void parseBlock(std::string& line) override {
        static std::vector<std::regex> hlRegex = {
            std::regex("^# (.*)"),
            std::regex("^(?:#){2} (.*)"),
            std::regex("^(?:#){3} (.*)"),
            std::regex("^(?:#){4} (.*)"),
            std::regex("^(?:#){5} (.*)"),
            std::regex("^(?:#){6} (.*)")
        };
        static std::vector<std::string> hlReplacement = {
            "<h1>$1</h1>",
            "<h2>$1</h2>",
            "<h3>$1</h3>",
            "<h4>$1</h4>",
            "<h5>$1</h5>",
            "<h6>$1</h6>"
        };
        for (uint8_t i = 0; i < 6; ++i) {
            line = std::regex_replace(line, hlRegex[i], hlReplacement[i]);
        }
    }
}; // class HeadlineParser
class HorizontalLineParser : public BlockParser {
    public:
        HorizontalLineParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        ) : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , lineRegex("^---$")
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^---$");
            return std::regex_match(line, re);
        }
        bool IsFinished() const override { return true; }
    protected:
        bool isInlineBlockAllowed() const override { return false; }
        bool isLineParserAllowed() const override { return false; }
        void parseBlock(std::string& line) override {
            static std::string replacement = "<hr/>";
            line = std::regex_replace(line, lineRegex, replacement);
        }
    private:
        std::regex lineRegex;
}; // class HorizontalLineParser
class ImageParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            static std::regex re("\\!\\[([^\\]]*)\\]\\(([^\\]]*)\\)");
            static std::string replacement = "<img src=\"$2\" alt=\"$1\"/>";
            line = std::regex_replace(line, re, replacement);
        }
}; // class ImageParser
class InlineCodeParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            static std::regex re("`([^`]*)`");
            static std::string replacement = "<code>$1</code>";
            line = std::regex_replace(line, re, replacement);
        }
}; // class InlineCodeParser
class ItalicParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            std::regex re("(?!.*`.*|.*<code>.*)\\*(?!.*`.*|.*<\\/code>.*)([^\\*]*)\\*(?!.*`.*|.*<\\/code>.*)");
            static std::string replacement = "<i>$1</i>";
            line = std::regex_replace(line, re, replacement);
        }
}; // class ItalicParser
class LineParser {
    public:
        virtual ~LineParser() {}
        virtual void Parse(std::string& line) = 0;
}; // class LineParser
class LinkParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            static std::regex re("\\[([^\\]]*)\\]\\(([^\\]]*)\\)");
            static std::string replacement = "<a href=\"$2\">$1</a>";
            line = std::regex_replace(line, re, replacement);
        }
}; // class LinkParser
class OrderedListParser : public BlockParser {
    public:
        OrderedListParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^1\\. .*");
            return std::regex_match(line, re);
        }
    bool IsFinished() const override {
        return this->isFinished;
    }
    protected:
        bool isInlineBlockAllowed() const override { return true; }
        bool isLineParserAllowed() const override { return true; }
        void parseBlock(std::string& line) override {
            bool isStartOfNewListItem = this->isStartOfNewListItem(line);
            uint32_t indentation = getIndentationWidth(line);
            static std::regex orderedlineRegex("^1\\. ");
            line = std::regex_replace(line, orderedlineRegex, "");
            static std::regex unorderedlineRegex("^(\\* )");
            line = std::regex_replace(line, unorderedlineRegex, "");
            if (!this->isStarted) {
                line = "<ol><li>" + line;
                this->isStarted = true;
                return;
            }
            if (indentation >= 2) {
                line = line.substr(2);
                return;
            }
            if (
                line.empty() ||
                line.find("</li><li>") != std::string::npos ||
                line.find("</li></ol>") != std::string::npos ||
                line.find("</li></ul>") != std::string::npos
            ) {
                line = "</li></ol>" + line;
                this->isFinished = true;
                return;
            }
            if (isStartOfNewListItem) {
                line = "</li><li>" + line;
            }
        }
    private:
        bool isStarted;
        bool isFinished;
        bool isStartOfNewListItem(const std::string& line) const {
            static std::regex re("^(?:1\\. |\\* ).*");
            return std::regex_match(line, re);
        }
}; // class OrderedListParser

class ParagraphParser : public BlockParser {
    public:
        ParagraphParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        {}
        static bool IsStartingLine(const std::string& line) {
            return !line.empty();
        }
        bool IsFinished() const override {
            return this->isFinished;
        }
    protected:
        bool isInlineBlockAllowed() const override { return false; }
        bool isLineParserAllowed() const override { return true; }
        void parseBlock(std::string& line) override {
            if (!this->isStarted) {
                line = "<p>" + line + " ";
                this->isStarted = true;
                return;
            }
            if (line.empty()) {
                line += "</p>";
                this->isFinished = true;
                return;
            }
            line += " ";
        }
    private:
        bool isStarted;
        bool isFinished;
}; // class ParagraphParser
class Parser {
    public:
        Parser() : emphasizedParser(std::make_shared<EmphasizedParser>())
        , imageParser(std::make_shared<ImageParser>())
        , inlineCodeParser(std::make_shared<InlineCodeParser>())
        , italicParser(std::make_shared<ItalicParser>())
        , linkParser(std::make_shared<LinkParser>())
        , strikeThroughParser(std::make_shared<StrikeThroughParser>())
        , strongParser(std::make_shared<StrongParser>())
        {}
        std::string Parse(std::stringstream& markdown) const {
            std::string result = "";
            std::shared_ptr<BlockParser> currentBlockParser = nullptr;
            for (std::string line; std::getline(markdown, line);) {
                if (!currentBlockParser) {
                    currentBlockParser = getBlockParserForLine(line);
                }
                if (currentBlockParser) {
                    currentBlockParser->AddLine(line);
                    if (currentBlockParser->IsFinished()) {
                        result += currentBlockParser->GetResult().str();
                        currentBlockParser = nullptr;
                    }
                }
            }
            // make sure, that all parsers are finished
            if (currentBlockParser) {
                std::string emptyLine = "";
                currentBlockParser->AddLine(emptyLine);
                if (currentBlockParser->IsFinished()) {
                    result += currentBlockParser->GetResult().str();
                    currentBlockParser = nullptr;
                }
            }
            return result;
        }
    private:
        std::shared_ptr<EmphasizedParser> emphasizedParser;
        std::shared_ptr<ImageParser> imageParser;
        std::shared_ptr<InlineCodeParser> inlineCodeParser;
        std::shared_ptr<ItalicParser> italicParser;
        std::shared_ptr<LinkParser> linkParser;
        std::shared_ptr<StrikeThroughParser> strikeThroughParser;
        std::shared_ptr<StrongParser> strongParser;
        // block parser have to run before
        void runLineParser(std::string& line) const {
            // Attention! ImageParser has to be before LinkParser
            this->imageParser->Parse(line);
            this->linkParser->Parse(line);
            // Attention! StrongParser has to be before EmphasizedParser
            this->strongParser->Parse(line);
            this->emphasizedParser->Parse(line);
            this->strikeThroughParser->Parse(line);
            this->inlineCodeParser->Parse(line);
            this->italicParser->Parse(line);
        }
        std::shared_ptr<BlockParser> getBlockParserForLine(const std::string& line) const {
            std::shared_ptr<BlockParser> parser;
            if (CodeBlockParser::IsStartingLine(line)) {
                parser = std::make_shared<CodeBlockParser>( nullptr, nullptr );
            } else if (HeadlineParser::IsStartingLine(line)) {
                parser = std::make_shared<HeadlineParser>( nullptr, nullptr );
            } else if (HorizontalLineParser::IsStartingLine(line)) {
                parser = std::make_shared<HorizontalLineParser>( nullptr, nullptr );
            } else if (QuoteParser::IsStartingLine(line)) {
                parser = std::make_shared<QuoteParser>(
                    [this](std::string& line){ this->runLineParser(line); },
                    [this](const std::string& line){ return this->getBlockParserForLine(line); }
                );
            } else if (TableParser::IsStartingLine(line)) {
                parser = std::make_shared<TableParser>(
                    [this](std::string& line){ this->runLineParser(line); },
                    nullptr
                );
            } else if (ChecklistParser::IsStartingLine(line)) {
                parser = this->createChecklistParser();
            } else if (OrderedListParser::IsStartingLine(line)) {
                parser = this->createOrderedListParser();
            } else if (UnorderedListParser::IsStartingLine(line)) {
                parser = this->createUnorderedListParser();
            } else if (ParagraphParser::IsStartingLine(line)) {
                parser = std::make_shared<ParagraphParser>(
                    [this](std::string& line){ this->runLineParser(line); },
                    nullptr
                );
            }
            return parser;
        }
        std::shared_ptr<BlockParser> createChecklistParser() const {
            return std::make_shared<ChecklistParser>(
                [this](std::string& line){ this->runLineParser(line); },
                [this](const std::string& line) {
                    std::shared_ptr<BlockParser> parser;
                    if (ChecklistParser::IsStartingLine(line)) {
                        parser = this->createChecklistParser();
                    }
                    return parser;
                }
            );
        }
        std::shared_ptr<BlockParser> createOrderedListParser() const {
            return std::make_shared<OrderedListParser>(
                [this](std::string& line){ this->runLineParser(line); },
                [this](const std::string& line) {
                    std::shared_ptr<BlockParser> parser;
                    if (OrderedListParser::IsStartingLine(line)) {
                        parser = this->createOrderedListParser();
                    } else if (UnorderedListParser::IsStartingLine(line)) {
                        parser = this->createUnorderedListParser();
                    }
                    return parser;
                }
            );
        }
        std::shared_ptr<BlockParser> createUnorderedListParser() const {
            return std::make_shared<UnorderedListParser>(
                [this](std::string& line){ this->runLineParser(line); },
                [this](const std::string& line) {
                    std::shared_ptr<BlockParser> parser;
                    if (OrderedListParser::IsStartingLine(line)) {
                        parser = this->createOrderedListParser();
                    } else if (UnorderedListParser::IsStartingLine(line)) {
                        parser = this->createUnorderedListParser();
                    }
                    return parser;
                }
            );
        }
}; // class Parser

class QuoteParser : public BlockParser {
    public:
        QuoteParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^\\>.*");
            return std::regex_match(line, re);
        }
        void AddLine(std::string& line) override {
            if (!this->isStarted) {
                this->result << "<blockquote>";
                this->isStarted = true;
            }
            bool finish = false;
            if (line.empty()) {
                finish = true;
            }
            this->parseBlock(line);
            if (this->isInlineBlockAllowed() && !this->childParser) {
                this->childParser = this->getBlockParserForLine(line);
            }
            if (this->childParser) {
                this->childParser->AddLine(line);
                if (this->childParser->IsFinished()) {
                    this->result << this->childParser->GetResult().str();
                    this->childParser = nullptr;
                }
                return;
            }
            if (this->isLineParserAllowed()) {
                this->parseLine(line);
            }
            if (finish) {
                this->result << "</blockquote>";
                this->isFinished = true;
            }
            this->result << line;
        }
        bool IsFinished() const override { return this->isFinished; }
    protected:
        bool isInlineBlockAllowed() const override { return true; }
        bool isLineParserAllowed() const override { return true; }
        void parseBlock(std::string& line) override {
            static std::regex lineRegexWithSpace("^\\> ");
            line = std::regex_replace(line, lineRegexWithSpace, "");
            static std::regex lineRegexWithoutSpace("^\\>");
            line = std::regex_replace(line, lineRegexWithoutSpace, "");
            if (!line.empty()) {
                line += " ";
            }
        }
    private:
        bool isStarted;
        bool isFinished;
}; // class QuoteParser

class StrikeThroughParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            static std::regex re("(?!.*`.*|.*<code>.*)\\~\\~(?!.*`.*|.*<\\/code>.*)([^\\~]*)\\~\\~(?!.*`.*|.*<\\/code>.*)");
            static std::string replacement = "<s>$1</s>";
            line = std::regex_replace(line, re, replacement);
        }
}; // class StrikeThroughParser

class StrongParser : public LineParser {
    public:
        void Parse(std::string& line) override {
            static std::vector<std::regex> res {
                std::regex{"(?!.*`.*|.*<code>.*)\\*\\*(?!.*`.*|.*<\\/code>.*)([^\\*\\*]*)\\*\\*(?!.*`.*|.*<\\/code>.*)"},
                std::regex{"(?!.*`.*|.*<code>.*)__(?!.*`.*|.*<\\/code>.*)([^__]*)__(?!.*`.*|.*<\\/code>.*)"}
            };
            static std::string replacement = "<strong>$1</strong>";
            for (const auto& re : res) {
                line = std::regex_replace(line, re, replacement);
            }
        }
}; // class StrongParser

class TableParser : public BlockParser {
    public:
        TableParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        , currentBlock(0)
        , currentRow(0)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::string matchString("|table>");
            return line == matchString;
        }
        void AddLine(std::string& line) override {
            if (!this->isStarted && line == "|table>") {
                this->isStarted = true;
                return;
            }
            if (this->isStarted) {
                if (line == "- | - | -") {
                    ++this->currentBlock;
                    this->currentRow = 0;
                    return;
                }
                if (line == "|<table") {
                    static std::string emptyLine = "";
                    this->parseBlock(emptyLine);
                    this->isFinished = true;
                    return;
                }
                if (this->table.size() < this->currentBlock + 1) {
                    this->table.push_back(std::vector<std::vector<std::string>>());
                }
                this->table[this->currentBlock].push_back(std::vector<std::string>());
                std::string segment;
                std::stringstream streamToSplit(line);
                while (std::getline(streamToSplit, segment, '|')) {
                    this->parseLine(segment);
                    this->table[this->currentBlock][this->currentRow].push_back(segment);
                }
                ++this->currentRow;
            }
        }
        bool IsFinished() const override { return this->isFinished; }
    protected:
        bool isInlineBlockAllowed() const override { return false; }
        bool isLineParserAllowed() const override { return true; }
        void parseBlock(std::string&) override {
            result << "<div class=\"responsive-table\"><table>";
            bool hasHeader = false;
            bool hasFooter = false;
            bool isFirstBlock = true;
            uint32_t currentBlockNumber = 0;
            if (this->table.size() > 1) {
                hasHeader = true;
            }
            if (this->table.size() >= 3) {
                hasFooter = true;
            }
            for (const std::vector<std::vector<std::string>>& block : this->table) {
                bool isInHeader = false;
                bool isInFooter = false;
                ++currentBlockNumber;
                if (hasHeader && isFirstBlock) {
                    result << "<thead>";
                    isInHeader = true;
                } else if (hasFooter && currentBlockNumber == this->table.size()) {
                    result << "<tfoot>";
                    isInFooter = true;
                } else {
                    result << "<tbody>";
                }
                for (const std::vector<std::string>& row : block) {
                    result << "<tr>";
                    for (const std::string& column : row) {
                        if (isInHeader) {
                            result << "<th>";
                        } else {
                            result << "<td>";
                        }
                        result << column;
                        if (isInHeader) {
                            result << "</th>";
                        } else {
                            result << "</td>";
                        }
                    }
                    result << "</tr>";
                }
                if (isInHeader) {
                    result << "</thead>";
                } else if (isInFooter) {
                    result << "</tfoot>";
                } else {
                    result << "</tbody>";
                }
                isFirstBlock = false;
            }
            result << "</table></div>";
        }
    private:
        bool isStarted;
        bool isFinished;
        uint32_t currentBlock;
        uint32_t currentRow;
        std::vector<std::vector<std::vector<std::string>>> table;
}; // class TableParser

class UnorderedListParser : public BlockParser {
    public:
        UnorderedListParser(
            std::function<void(std::string&)> parseLineCallback,
            std::function<std::shared_ptr<BlockParser>(const std::string& line)> getBlockParserForLineCallback
        )
        : BlockParser(parseLineCallback, getBlockParserForLineCallback)
        , isStarted(false)
        , isFinished(false)
        {}
        static bool IsStartingLine(const std::string& line) {
            static std::regex re("^\\* .*");
            return std::regex_match(line, re);
        }
        bool IsFinished() const override { return this->isFinished; }
    protected:
        bool isInlineBlockAllowed() const override { return true; }
        bool isLineParserAllowed() const override { return true; }
        void parseBlock(std::string& line) override {
            bool isStartOfNewListItem = IsStartingLine(line);
            uint32_t indentation = getIndentationWidth(line);
            static std::regex lineRegex("^(\\* )");
            line = std::regex_replace(line, lineRegex, "");
            if (!this->isStarted) {
                line = "<ul><li>" + line;
                this->isStarted = true;
                return;
            }
            if (indentation >= 2) {
                line = line.substr(2);
                return;
            }
            if (
                line.empty() ||
                line.find("</li><li>") != std::string::npos ||
                line.find("</li></ol>") != std::string::npos ||
                line.find("</li></ul>") != std::string::npos
            ) {
                line = "</li></ul>" + line;
                this->isFinished = true;
                return;
            }
            if (isStartOfNewListItem) {
                line = "</li><li>" + line;
            }
        }
    private:
        bool isStarted;
        bool isFinished;
}; // class UnorderedListParser

#endif