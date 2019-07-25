#pragma once

#include "sf_template.h"
#include "sf_utils.hpp"
#include "sf_json.hpp"

#include "sf_stdc++.h"
#include "sf_logger.hpp"

#include "sf_lex.hpp"

namespace skyfire
{
    inline std::string sf_render(std::string content, const sf_json &js, const std::pair<std::string,std::string> &sep) {
        auto back_up = content;

        auto extract_result = sf_extract__(content, sep);
        for (auto p = extract_result.rbegin(); p != extract_result.rend(); ++p) {
            sf_template_make_tree__(p->data, js);
            // TODO replace text
        }

        return content;
    }

    inline std::vector<sf_template_info_t> sf_extract__(const std::string &content,
                                                        const std::pair<std::string, std::string> &sep)
    {
        std::vector<sf_template_info_t> ret;
        auto start = sep.first;
        auto end = sep.second;
        int pos = 0;
        while(true)
        {
            pos = content.find(start, pos);
            if(pos == -1)
            {
                break;
            }
            auto s_pos = pos + start.length();
            pos = content.find(end, s_pos);
            if(pos == -1)
            {
                break;
            }
            auto e_pos = pos;
            pos += end.length();
            sf_template_info_t tmp;
            tmp.data = {content.substr(s_pos, e_pos-s_pos)};
            tmp.pos = s_pos;
            tmp.len = e_pos-s_pos;
            ret.push_back(tmp);
        }
        return ret;
    }

    inline std::shared_ptr<sf_template_tree_node_t> sf_template_make_tree__(std::string content, const sf_json &js) {
        sf_lex lex;
        lex.set_rules({
                              {"if",       R"(if)"},
                              {"sp",       R"(\s+)"},
                              {"for",      R"(for)"},
                              {"in",       R"(in)"},
                              {"print",    R"(print)"},
                              {"string",   R"("([^\\"]|(\\["\\/bnrt]|(u[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F])))*")"},
                              {"number",   R"(-?(0|[1-9]\d*)(\.\d+)?(e|E(\+|-)?0|[1-9]\d*)?)"},
                              {"true",     R"(true)"},
                              {"false",    R"(false)"},
                              {"null",     R"(null)"},
                              {":",        R"(:)"},
                              {"(",        R"(\()"},
                              {")",        R"(\))"},
                              {"{",        R"(\{)"},
                              {"}",        R"(\})"},
                              {"[",        R"(\[)"},
                              {"]",        R"(\])"},
                              {"\"",       R"(")"},
                              {";",        R"(;)"},
                              {",",        R"(,)"},
                              {".",        R"(\.)"},
                              {"+",        R"(\+)"},
                              {"-",        R"(\-)"},
                              {"*",        R"(\*)"},
                              {"/",        R"(/)"},
                              {"%",        R"(%)"},
                              {"=",        R"(=)"},
                              {"!",        R"(\!)"},
                              {"!=",       R"(\!=)"},
                              {"==",       R"(==)"},
                              {"&&",       R"(\&\&)"},
                              {"||",       R"(\|\|)"},
                              {"+=",       R"(\+=)"},
                              {"-=",       R"(\-=)"},
                              {"*=",       R"(\*=)"},
                              {"/=",       R"(/=)"},
                              {"%=",       R"(%=)"},
                              {"&&=",      R"(\&\&=)"},
                              {"||=",      R"(\|\|=)"},

                              {"key_word", R"(\w+)"},
                      });

        sf_yacc yacc;

        auto value_callback= [](const std::vector<std::shared_ptr<sf_yacc_result_t>> & d)->std::any{
            auto tmp = std::make_shared<sf_template_tree_node_t>();
            tmp->operation = "value";
            tmp->data = d[0]->text;
            return tmp;
        };

        auto two_op_callback = [](const std::string &op, const std::vector<std::shared_ptr<sf_yacc_result_t>> & d)->std::any {
            auto tmp = std::make_shared<sf_template_tree_node_t>();
            tmp->operation=op;
            tmp->param.push_back(d[0]->user_data);
            tmp->param.push_back(d[2]->user_data);
            return tmp;
        };

        yacc.set_rules(
                {
                        {
                                "value",
                                {
                                        {
                                                {"true"},
                                                value_callback
                                        },
                                        {
                                                {"false"},
                                                value_callback
                                        },
                                        {
                                                {"null"},
                                                value_callback
                                        },
                                        {
                                                {"key_word"},
                                                value_callback
                                        },
                                        {
                                                {"string"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "value";
                                                    tmp->data = sf_json::json_string_to_string(d[0]->text);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"number"},
                                                value_callback
                                        },
                                        {
                                                {"int"},
                                                value_callback
                                        },
                                        {
                                                {"(", "value", ")"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return d[1]->user_data;
                                                }
                                        },
                                        {
                                                {"value", "[", "string", "]"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "[]";
                                                    tmp->param.push_back(d[0]->user_data);
                                                    tmp->param.push_back(d[2]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"!", "value"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "!";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"value", "!=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("!=", d);
                                                }
                                        },
                                        {
                                                {"value", "==", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("==", d);
                                                }
                                        },
                                        {
                                                {"value", ".", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("[]", d);
                                                }
                                        },
                                        {
                                                {"value", "+", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("+", d);
                                                }
                                        },
                                        {
                                                {"value", "-", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("-", d);
                                                }
                                        },
                                        {
                                                {"value", "*", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("*", d);
                                                }
                                        },
                                        {
                                                {"value", "/", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("/", d);
                                                }
                                        },
                                        {
                                                {"value", "%", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("%", d);
                                                }
                                        },
                                        {
                                                {"value", "=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("=", d);
                                                }
                                        },
                                        {
                                                {"-", "value"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "-";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"value", "&&", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("&&", d);
                                                }
                                        },
                                        {
                                                {"value", "||", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("||", d);
                                                }
                                        },
                                        {
                                                {"value", "+=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("+=", d);
                                                }
                                        },
                                        {
                                                {"value", "-=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("-=", d);
                                                }
                                        },
                                        {
                                                {"value", "*=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("*=", d);
                                                }
                                        },
                                        {
                                                {"value", "/=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("/=", d);
                                                }
                                        },
                                        {
                                                {"value", "%=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("%=", d);
                                                }
                                        },
                                        {
                                                {"value", "&&=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("&&=", d);
                                                }
                                        },
                                        {
                                                {"value", "||=", "value"},
                                                [=](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("||=", d);
                                                }
                                        },
                                        {
                                                {"+", "value"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return d[1]->user_data;
                                                }
                                        }
                                }
                        },
                        {
                                "for_block",
                                {
                                        {
                                                {"for", "key_word", "in", "value", "block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "for_in";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    tmp->param.push_back(d[3]->user_data);
                                                    tmp->param.push_back(d[4]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"for", "key_word", "key_word", "in", "value", "block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "for_in";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    tmp->param.push_back(d[2]->user_data);
                                                    tmp->param.push_back(d[4]->user_data);
                                                    tmp->param.push_back(d[5]->user_data);
                                                    return tmp;
                                                }
                                        },

                                        {
                                                {"for", "value", ";", "value", ";", "value", "block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "for_loop";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    tmp->param.push_back(d[3]->user_data);
                                                    tmp->param.push_back(d[5]->user_data);
                                                    tmp->param.push_back(d[6]->user_data);
                                                    return tmp;
                                                }
                                        }
                                }
                        },
                        {
                                "block",
                                {
                                        {
                                                {"{",   "}"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "block";
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"{",        "sentences", "}"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "block";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    return tmp;
                                                }
                                        }
                                }
                        },
                        {
                                "sentence",
                                {
                                        {
                                                {"value"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "sentence";
                                                    tmp->param.push_back(d[0]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"for_block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "sentence";
                                                    tmp->param.push_back(d[0]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"if_block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "sentence";
                                                    tmp->param.push_back(d[0]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "sentence";
                                                    tmp->param.push_back(d[0]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"sentence", "value"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[0]->user_data);
                                                    tmp->param.push_back(d[1]->user_data);
                                                    return tmp;
                                                }
                                        }
                                }
                        },
                        {
                                "if_block",
                                {
                                        {
                                                {"if",  "value",    "block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "if";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    tmp->param.push_back(d[2]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"if",       "value",     "block", "else",  "block"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "if";
                                                    tmp->param.push_back(d[1]->user_data);
                                                    tmp->param.push_back(d[2]->user_data);
                                                    tmp->param.push_back(d[4]->user_data);
                                                    return tmp;
                                                }
                                        }
                                }
                        },
                        {
                                "language",
                                {
                                        {
                                                {"sentence"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "language";
                                                    tmp->param.push_back(d[0]->user_data);
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"language", "sentence"},
                                                [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    auto tmp = std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[0]->user_data);
                                                    tmp->param.push_back(d[1]->user_data);
                                                    return tmp;
                                                }
                                        }
                                }
                        }
                }
        );

        yacc.add_terminate_ids({"language"});

        std::vector<sf_lex_result_t> lex_result;
        auto lex_res = lex.parse(content, lex_result);
        if (!lex_res) {
            return nullptr;
        }
        lex_result.erase(std::remove_if(lex_result.begin(), lex_result.end(), [](const sf_lex_result_t &t) {
            return t.id == "sp";
        }), lex_result.end());

        for(auto &p:lex_result)
        {
            std::cout<<p.id<<"【"<<p.matched_str<<"】";
        }
        std::cout << std::endl;

        std::vector<std::shared_ptr<sf_yacc_result_t>> yacc_result;
        auto yacc_res = yacc.parse(lex_result, yacc_result);

        if(!yacc_res){
            return nullptr;
        }

        for(auto &p:yacc_result)
        {
            std::cout<<std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(p->user_data)->operation;
        }
        return nullptr;
    }
}