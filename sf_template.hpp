#pragma once

#include "sf_template.h"
#include "sf_utils.hpp"
#include "sf_json.hpp"

#include "sf_stdc++.h"
#include "sf_logger.hpp"

#include "sf_lex.hpp"

namespace skyfire {
    inline std::string
    sf_render(std::string content, const sf_json &js, const std::pair<std::string, std::string> &sep) {
        auto back_up = content;

        auto extract_result = sf_extract__(content, sep);
        for (auto p = extract_result.rbegin(); p != extract_result.rend(); ++p) {
            auto root = sf_template_make_tree__(p->data, js);
            std::string result = "";
            if(root) {
                auto js_clone = js.clone();
                result = sf_template_exec__(root, js_clone);
            }
            content.replace(p->pos-sep.first.size(), p->len+sep.first.size()+sep.second.size(), result);
        }

        return content;
    }

    inline std::vector<sf_template_info_t> sf_extract__(const std::string &content,
                                                        const std::pair<std::string, std::string> &sep) {
        std::vector<sf_template_info_t> ret;
        auto start = sep.first;
        auto end = sep.second;
        int pos = 0;
        while (true) {
            pos = content.find(start, pos);
            if (pos == -1) {
                break;
            }
            auto s_pos = pos + start.length();
            pos = content.find(end, s_pos);
            if (pos == -1) {
                break;
            }
            auto e_pos = pos;
            pos += end.length();
            sf_template_info_t tmp;
            tmp.data = {content.substr(s_pos, e_pos - s_pos)};
            tmp.pos = s_pos;
            tmp.len = e_pos - s_pos;
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


		auto make_empty = [](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d)
		{
			for(auto &p: d)
			{
				try
				{
					std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(p->user_data);
				}
				catch(std::bad_cast e)
				{
					auto t = std::make_shared<sf_template_tree_node_t>();
					t->operation = p->id;
					t->data = p->text;
					p->user_data = t;
				}
			}
		};


        auto value_callback = [make_empty](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
			make_empty(d);
            auto tmp = std::make_shared<sf_template_tree_node_t>();
            tmp->operation = "value";
            tmp->data = d[0]->text;
            return tmp;
        };

        auto two_op_callback = [make_empty](const std::string &op,
                                  const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
			make_empty(d);
            auto tmp = std::make_shared<sf_template_tree_node_t>();
            tmp->operation = op;
            tmp->param.push_back(std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[0]->user_data));
            tmp->param.push_back(std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[2]->user_data));
            return tmp;
        };

        auto sentence_callback = [make_empty](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
            make_empty(d);
            auto tmp = std::make_shared<sf_template_tree_node_t>();
            tmp->operation = "sentence";
            tmp->param.push_back(std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[0]->user_data));
            tmp->param.push_back(std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[1]->user_data));
            return tmp;
        };


        auto merge_sentence_callback = [make_empty](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
            make_empty(d);
            auto tmp = std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[1]->user_data);
            tmp->param.insert(tmp->param.begin(),std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[0]->user_data));
            return tmp;
        };

		auto block_callback = [make_empty](const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
            make_empty(d);
            auto tmp = std::make_shared<sf_template_tree_node_t>();
            tmp->operation = "block";
            tmp->param.push_back(std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(d[1]->user_data));
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
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
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
                                                {"value",     "[", "string", "]"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "[]";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[0]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[2]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"!",         "value"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "!";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"value",    "!=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("!=", d);
                                                }
                                        },
                                        {
                                                {"value",    "==", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("==", d);
                                                }
                                        },
                                        {
                                                {"value",    ".", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("[]", d);
                                                }
                                        },
                                        {
                                                {"value",    "+", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("+", d);
                                                }
                                        },
                                        {
                                                {"value", "-", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("-", d);
                                                }
                                        },
                                        {
                                                {"value", "*", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("*", d);
                                                }
                                        },
                                        {
                                                {"value", "/", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("/", d);
                                                }
                                        },
                                        {
                                                {"value", "%", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("%", d);
                                                }
                                        },
                                        {
                                                {"-",        "value"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "-";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"value",    "&&", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("&&", d);
                                                }
                                        },
                                        {
                                                {"value",    "||", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("||", d);
                                                }
                                        },
                                        {
                                                {"+",        "value"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    return d[1]->user_data;
                                                }
                                        },
                                        {
                                                {"key_word", "(", "param", ")"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "func";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[0]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[2]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"key_word", "(", "value", ")"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "func";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[0]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[2]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"key_word", "(", ")"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "func";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[0]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"value", "=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("=", d);
                                                }
                                        },

                                        {
                                                {"value", "+=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("+=", d);
                                                }
                                        },
                                        {
                                                {"value", "-=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("-=", d);
                                                }
                                        },
                                        {
                                                {"value", "*=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("*=", d);
                                                }
                                        },
                                        {
                                                {"value", "/=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("/=", d);
                                                }
                                        },
                                        {
                                                {"value", "%=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("%=", d);
                                                }
                                        },
                                        {
                                                {"value", "&&=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("&&=", d);
                                                }
                                        },
                                        {
                                                {"value", "||=", "value"},
                                                [two_op_callback](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    return two_op_callback("||=", d);
                                                }
                                        }
                                }
                        },
                        {
                                "param",
                                {       {
                                                {"value", ",",        "value"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "param";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[0]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[2]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"value", ",",        "param"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                            d[2]->user_data);
                                                    tmp->param.insert(tmp->param.begin(),
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[0]->user_data));
                                                    return tmp;
                                                }
                                        }

                                }
                        },
                        {
                                "for_block",
                                {
                                        {
                                                {"for",   "key_word", "in", "value", "block"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "for_in";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[3]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[4]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"for",   "key_word", ",",     "key_word", "in", "value", "block"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "for_in";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[3]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[5]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[6]->user_data));
                                                    return tmp;
                                                }
                                        },

                                        {
                                                {"for",   "value",    ";", "value", ";", "value", "block"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "for_loop";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[3]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[5]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[6]->user_data));
                                                    return tmp;
                                                }
                                        }
                                }
                        },
                        {
                                "block",
                                {
                                        {
                                                {"{",     "}"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "block";
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"{",     "value",    "}"},
                                                block_callback
                                        },
                                        {
                                                {"{",     "sentence", "}"},
                                                block_callback
                                        },
                                        {
                                                {"{",     "for_block", "}"},
                                                block_callback
                                        },
                                        {
                                                {"{",         "if_block", "}"},
                                                block_callback
                                        },
                                        {
                                                {"{",         "block", "}"},
                                                block_callback
                                        }
                                }
                        },
                        {
                                "sentence",
                                {
                                        {
                                                {"value", "value"},
                                                sentence_callback
                                        },
                                        {
                                                {"value", "for_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"value", "if_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"value", "block"},
                                                sentence_callback
                                        },
                                        {
                                                {"for_block", "value"},
                                                sentence_callback
                                        },
                                        {
                                                {"for_block", "for_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"for_block", "if_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"for_block", "block"},
                                                sentence_callback
                                        },
                                        {
                                                {"if_block", "value"},
                                                sentence_callback
                                        },
                                        {
                                                {"if_block", "for_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"if_block", "if_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"if_block", "block"},
                                                sentence_callback
                                        },
                                        {
                                                {"block", "value"},
                                                sentence_callback
                                        },
                                        {
                                                {"block", "for_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"block", "if_block"},
                                                sentence_callback
                                        },
                                        {
                                                {"block", "block"},
                                                sentence_callback
                                        },
                                        {
                                                {"value", "sentence"},
                                                merge_sentence_callback
                                        },
                                        {
                                                {"for_block", "sentence"},
                                                merge_sentence_callback
                                        },
                                        {
                                                {"if_block", "sentence"},
                                                merge_sentence_callback
                                        },
                                        {
                                                {"block", "sentence"},
                                                merge_sentence_callback
                                        },
                                }
                        },
                        {
                                "if_block",
                                {
                                        {
                                                {"if",    "value",    "block"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "if";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[2]->user_data));
                                                    return tmp;
                                                }
                                        },
                                        {
                                                {"if",    "value",    "block", "else",     "block"},
                                                [make_empty](
                                                        const std::vector<std::shared_ptr<sf_yacc_result_t>> &d) -> std::any {
                                                    make_empty(d);
                                                    auto tmp = std::make_shared<sf_template_tree_node_t>();
                                                    tmp->operation = "if";
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[1]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[2]->user_data));
                                                    tmp->param.push_back(
                                                            std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(
                                                                    d[4]->user_data));
                                                    return tmp;
                                                }
                                        }
                                }
                        }

                }
        );

        yacc.add_terminate_ids({"sentence", "if_block", "for_block", "block", "value"});

        std::vector<sf_lex_result_t> lex_result;
        auto lex_res = lex.parse(content, lex_result);
        if (!lex_res) {
            return nullptr;
        }
        std::cout << "lex pass" << std::endl;
        lex_result.erase(std::remove_if(lex_result.begin(), lex_result.end(), [](const sf_lex_result_t &t) {
            return t.id == "sp";
        }), lex_result.end());

        for (auto &p:lex_result) {
            std::cout << p.id << "<" << p.matched_str << ">";
        }
        std::cout << std::endl;

        std::vector<std::shared_ptr<sf_yacc_result_t>> yacc_result;
        auto yacc_res = yacc.parse(lex_result, yacc_result);

        if (!yacc_res) {
            return nullptr;
        }
        std::cout << "yacc pass" << yacc_result.size() << std::endl;
        for (auto &p:yacc_result) {
            std::cout << p->id << std::endl;
        }

        return std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(yacc_result[0]->user_data);
    }

    std::string sf_template_exec__(const std::shared_ptr<sf_template_tree_node_t> &root, sf_json &js) {
        print_tree__(root, 0);
        return "";
    }

    void print_tree__(const std::shared_ptr<sf_template_tree_node_t> &root, int index) {
        if(root->param.size() == 0)
        {
            std::cout<<"\""<<index<<"\"[label=\""<<root->operation<<"\"]"<<std::endl;
            std::cout<<"\""<<index*10+1<<"\"[label=\""<<root->data<<"\"]"<<std::endl;

            std::cout<<"\""<<index<<"\"->\""<<index*10+1<<"\""<<std::endl;

        } else{
            for(int i =0;i<root->param.size();++i)
            {
                auto t = std::any_cast<std::shared_ptr<sf_template_tree_node_t>>(root->param[i]);
                std::cout<<"\""<<index<<"\"[label=\""<<root->operation<<"\"]"<<std::endl;
                std::cout<<"\""<<index<<"\"->\""<<index*10+i+1<<"\""<<std::endl;
                print_tree__(t, index*10+i+1);
            }
        }
    }
}