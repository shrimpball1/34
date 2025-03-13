#include "StringUtils.h"

namespace StringUtils{

std::string Slice(const std::string &str, ssize_t start, ssize_t end) noexcept{
    ssize_t len = str.length();
    if(start < 0)start += len;
    if(end <= 0)end += len;
    start = std::max(static_cast<ssize_t>(0), std::min(start, len));
    end = std::max(static_cast<ssize_t>(0), std::min(end, len));
    return str.substr(start, end - start);
}

std::string Capitalize(const std::string &str) noexcept{
    std::string result = str;
    result[0] = std::toupper(result[0]);
    for(size_t i = 1; i < result.size(); ++i){
        result[i] = std::tolower(result[i]);
    }
    return result;
}

std::string Upper(const std::string &str) noexcept{
    std::string result = str;
    for(size_t i = 0; i < result.size(); i++){
        result[i] = std::toupper(result[i]);
    }
    return result;
}

std::string Lower(const std::string &str) noexcept{
    std::string result = str;
    for(size_t i = 0; i < result.size(); i++){
        result[i] = std::tolower(result[i]);
    }
    return result;
}

std::string LStrip(const std::string &str) noexcept{
    size_t start = 0;
    while(start < str.size() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')){
        ++start;
    }
    return str.substr(start);
}

std::string RStrip(const std::string &str) noexcept{
    size_t end = str.size();
    while(end > 0 && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\n' || str[end - 1] == '\r')){
        --end;
    }
    return str.substr(0, end);
}


std::string Strip(const std::string &str) noexcept{
    return LStrip(RStrip(str));
}

std::string Center(const std::string &str, int width, char fill) noexcept{
    int pad = std::max(0, width - static_cast<int>(str.size()));
    int left_pad = pad / 2;
    int right_pad = pad - left_pad;
    return std::string(left_pad, fill) + str + std::string(right_pad, fill);
}

std::string LJust(const std::string &str, int width, char fill) noexcept{
    int pad = std::max(0, width - static_cast<int>(str.size()));
    return str + std::string(pad, fill);
}

std::string RJust(const std::string &str, int width, char fill) noexcept{
    int pad = std::max(0, width - static_cast<int>(str.size()));
    return std::string(pad, fill) + str;
}

std::string Replace(const std::string &str, const std::string &old, const std::string &rep) noexcept{
    std::string result;
    size_t i = 0;
    while(i < str.size()){
        bool match = true;
        for(size_t j = 0; j < old.size(); ++j){
            if(i + j >= str.size() || str[i + j] != old[j]){
                match = false;
                break;
            }
        }

        if(match){
            result += rep;
            i += old.size();
        }else{
            result += str[i];
            ++i;
        }
    }
    return result;
}


std::vector< std::string > Split(const std::string &str, const std::string &splt) noexcept{
    
    if(splt.empty()){
        std::vector<std::string> result;
        size_t i = 0;
        while(i < str.size()){
            // skip leading spaces
            while(i < str.size() && std::isspace(static_cast<unsigned char>(str[i])))
                ++i;
            if (i == str.size()) break;
            size_t j = i;
            while(j < str.size() && !std::isspace(static_cast<unsigned char>(str[j])))
                ++j;
            result.push_back(str.substr(i, j - i));
            i = j;
        }
        return result;
    }else{
        std::vector<std::string> result;
        size_t start = 0, end = 0;
        while((end = str.find(splt, start)) != std::string::npos){
            result.push_back(str.substr(start, end - start));
            start = end + splt.size();
        }
        result.push_back(str.substr(start));
        return result;
    }
}


std::string Join(const std::string &str, const std::vector< std::string > &vect) noexcept{
    std::string result;
    for(size_t i = 0; i < vect.size(); ++i){
        if(i > 0){
            result += str;  
        }
        result += vect[i];
    }
    return result;
}

std::string ExpandTabs(const std::string &str, int tabsize) noexcept{
    std::string result;
    int pos = 0;

    if(tabsize <= 0){
        for(char ch : str){
            if(ch != '\t'){
                result.push_back(ch);
            }
        }
        return result; 
    }
    
    for(size_t i = 0; i < str.size(); ++i){
        if(str[i] == '\t') {
            int spaces = tabsize - (pos % tabsize);
            result += std::string(spaces, ' ');
            pos += spaces;
        }else{
            result += str[i];
            if(str[i] == '\n'){
                pos = 0;
            }else{
                ++pos;
            }
        }
    }

    return result;
}


int EditDistance(const std::string &left, const std::string &right, bool ignorecase) noexcept{
    std::string l = left, r = right;

    if(ignorecase){
        l = Lower(l);
        r = Lower(r);
    }

    size_t m = l.size();
    size_t n = r.size();

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for(size_t i = 0; i <= m; ++i){
        dp[i][0] = i;
    }
    for(size_t j = 0; j <= n; ++j){
        dp[0][j] = j;
    }

    for(size_t i = 1; i <= m; ++i){
        for(size_t j = 1; j <= n; ++j){
            if(l[i - 1] == r[j - 1]){
                dp[i][j] = dp[i - 1][j - 1];
            }else{
                dp[i][j] = std::min(
                    dp[i - 1][j - 1] + 1,
                    std::min(dp[i - 1][j] + 1, dp[i][j - 1] + 1)
                );
            }
        }
    }

    return dp[m][n];
}

};