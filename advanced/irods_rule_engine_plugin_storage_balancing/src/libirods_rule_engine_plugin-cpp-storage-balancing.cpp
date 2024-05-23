#include <irods/genQuery.h>
#include <irods/irods_file_object.hpp>
#include <irods/irods_kvp_string_parser.hpp>
#include <irods/irods_plugin_context.hpp>
#include <irods/irods_re_plugin.hpp>
#include <irods/irods_re_serialization.hpp>
#include <irods/irods_re_ruleexistshelper.hpp>
#include <irods/msParam.h>

#include <map>
#include <string>
#include <typeinfo>

#include <boost/coroutine/all.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

int rsGenQuery(rsComm_t*, genQueryInp_t*, genQueryOut_t**);

namespace {
    class GenQueryInpManager {
        genQueryInp_t& genquery_in;
    public:
        explicit GenQueryInpManager(genQueryInp_t& genquery_in) : genquery_in(genquery_in) {}
        ~GenQueryInpManager() {clearGenQueryInp(&genquery_in);}
    };

    class GenQueryOutManager {
        genQueryOut_t*& genquery_out;
    public:
        explicit GenQueryOutManager(genQueryOut_t*& genquery_out) : genquery_out(genquery_out) {}
        ~GenQueryOutManager() {freeGenQueryOut(&genquery_out);}
    };

    using GenQueryResultMap = std::map<int, char*>;

    auto make_genquery_coroutine(genQueryInp_t& genquery_in, rsComm_t& rs_comm) {
        boost::coroutines::asymmetric_coroutine<const GenQueryResultMap&>::pull_type source(
            [&genquery_in, &rs_comm](boost::coroutines::asymmetric_coroutine<const GenQueryResultMap&>::push_type& sink) {
                genQueryOut_t* genquery_out{nullptr};
                do {
                    const int status{rsGenQuery(&rs_comm, &genquery_in, &genquery_out)};
                    if (status == CAT_NO_ROWS_FOUND) {
                        break;
                    } else if (status < 0) {
                        boost::throw_exception(irods::exception(status, "rsGenQuery failed", __FILE__, __LINE__, __PRETTY_FUNCTION__));
                    }
                    GenQueryOutManager genquery_out_manager{genquery_out};
                    for (int i=0; i<genquery_out->rowCnt; ++i) {
                        GenQueryResultMap ret;
                        for (int j=0; j<genquery_out->attriCnt; ++j) {
                            sqlResult_t& result{genquery_out->sqlResult[j]};
                            ret[result.attriInx] = &result.value[result.len*i];
                        }
                        sink(ret);
                    }
                    genquery_in.continueInx = genquery_out->continueInx;
                } while (genquery_in.continueInx > 0);
            }, boost::coroutines::attributes(1000000));
        return source;
    }

    std::string get_resource_type(const std::string& resource_name, rsComm_t& rs_comm) {
        genQueryInp_t gen_inp;
        memset(&gen_inp, 0, sizeof(gen_inp));
        gen_inp.maxRows = MAX_SQL_ROWS;
        addInxIval(&gen_inp.selectInp, COL_R_TYPE_NAME, 1);
        std::stringstream constraint;
        constraint << "='" << resource_name << "'";
        addInxVal(&gen_inp.sqlCondInp, COL_R_RESC_NAME, constraint.str().c_str());
        GenQueryInpManager gen_inp_manager{gen_inp};
        auto query{make_genquery_coroutine(gen_inp, rs_comm)};
        for (const auto& result : query) {
            return result.at(COL_R_TYPE_NAME);
        }
        THROW(-1, boost::format("Failed to find resource type for resource: %s") % resource_name);
    }

    std::string get_resource_id_string(const std::string& resource_name, rsComm_t& rs_comm) {
        genQueryInp_t gen_inp;
        memset(&gen_inp, 0, sizeof(gen_inp));
        gen_inp.maxRows = MAX_SQL_ROWS;
        addInxIval(&gen_inp.selectInp, COL_R_RESC_ID, 1);
        std::stringstream constraint;
        constraint << "='" << resource_name << "'";
        addInxVal(&gen_inp.sqlCondInp, COL_R_RESC_NAME, constraint.str().c_str());
        GenQueryInpManager gen_inp_manager{gen_inp};
        auto query{make_genquery_coroutine(gen_inp, rs_comm)};
        for (const auto& result : query) {
            return result.at(COL_R_RESC_ID);
        }
        THROW(-1, boost::format("Failed to find resource type for resource: %s") % resource_name);
    }

    uint64_t get_bytes_used_by_leaf_resource(const std::string& resource_name, rsComm_t& rs_comm) {
        genQueryInp_t gen_inp;
        memset(&gen_inp, 0, sizeof(gen_inp));
        gen_inp.maxRows = MAX_SQL_ROWS;
        addInxIval(&gen_inp.selectInp, COL_DATA_SIZE, SELECT_SUM);
        std::stringstream constraint;
        constraint << "='" << resource_name << "'";
        addInxVal(&gen_inp.sqlCondInp, COL_R_RESC_NAME, constraint.str().c_str());
        GenQueryInpManager gen_inp_manager{gen_inp};
        auto query{make_genquery_coroutine(gen_inp, rs_comm)};
        uint64_t bytes_used{0};
        for (const auto& result : query) {
            if (strcmp(result.at(COL_DATA_SIZE), "")) {
                try {
                    bytes_used += boost::lexical_cast<uint64_t>(result.at(COL_DATA_SIZE));
                } catch (const boost::bad_lexical_cast&) {
                    THROW(-1, boost::format("Failed to cast sum(COL_DATA_SIZE) to uint64_t: %s") % result.at(COL_DATA_SIZE));
                }
            }
        }
        return bytes_used;
    }

    uint64_t get_bytes_used_by_all_children(const std::string& resource_name, rsComm_t& rs_comm) {
        const std::string resource_id_string{get_resource_id_string(resource_name, rs_comm)};
        genQueryInp_t gen_inp;
        memset(&gen_inp, 0, sizeof(gen_inp));
        gen_inp.maxRows = MAX_SQL_ROWS;
        addInxIval(&gen_inp.selectInp, COL_R_RESC_NAME, 1);
        std::stringstream constraint;
        constraint << "='" << resource_id_string << "'";
        addInxVal(&gen_inp.sqlCondInp, COL_R_RESC_PARENT, constraint.str().c_str());
        GenQueryInpManager gen_inp_manager{gen_inp};
        auto query{make_genquery_coroutine(gen_inp, rs_comm)};
        uint64_t bytes_used{0};
        for (const auto& result : query) {
            bytes_used += get_bytes_used_by_leaf_resource(result.at(COL_R_RESC_NAME), rs_comm);
        }
        return bytes_used;
    }

    uint64_t get_bytes_of_incoming_data_object(irods::plugin_context& plugin_context) {
        irods::file_object_ptr file_object_ptr{boost::dynamic_pointer_cast<irods::file_object>(plugin_context.fco())};
        if (file_object_ptr == nullptr) {
            auto raw_fco_pointer{plugin_context.fco().get()};
            THROW(-1, boost::format("failed to cast fco to file object: %s") % typeid(*raw_fco_pointer).name());
        }
        return file_object_ptr->size();
    }

    ruleExecInfo_t& get_rei(irods::callback& _effect_handler) {
        ruleExecInfo_t* rei{nullptr};
        irods::error ret{_effect_handler("unsafe_ms_ctx", &rei)};
        if (!ret.ok()) {
            THROW(ret.code(), "failed to get rei");
        }
        return *rei;
    }

    std::string get_resource_context_string(const std::string& resource_name, rsComm_t& rs_comm) {
        genQueryInp_t gen_inp;
        memset(&gen_inp, 0, sizeof(gen_inp));
        gen_inp.maxRows = MAX_SQL_ROWS;
        addInxIval(&gen_inp.selectInp, COL_R_RESC_CONTEXT, 1);
        std::stringstream constraint;
        constraint << "='" << resource_name << "'";
        addInxVal(&gen_inp.sqlCondInp, COL_R_RESC_NAME, constraint.str().c_str());
        GenQueryInpManager gen_inp_manager{gen_inp};
        auto query{make_genquery_coroutine(gen_inp, rs_comm)};
        for (const auto& result : query) {
            return result.at(COL_R_RESC_CONTEXT);
        }
        THROW(-1, boost::format("Failed to get context for resource: %s") % resource_name);
    }

    boost::optional<uint64_t> get_max_bytes(const std::string& resource_name, rsComm_t& rs_comm) {
        const std::string context{get_resource_context_string(resource_name, rs_comm)};
        if (context.empty()) {
            return boost::none;
        }
        irods::kvp_map_t kvp;
        const irods::error ret{irods::parse_kvp_string(context, kvp)};
        if (!ret.ok()) {
            THROW(ret.code(), boost::format("failed to parse resource [%s] context string: %s") % resource_name % context);
        }
        auto it{kvp.find("max_bytes")};
        if (it == kvp.end()) {
            return boost::none;
        }
        try {
            return boost::lexical_cast<uint64_t>(it->second);
        } catch (const boost::bad_lexical_cast&) {
            THROW(-1, boost::format("Failed to cast max_bytes to uint64_t: %s") % it->second);
        }
    }
}

static
irods::error start(irods::default_re_ctx&, const std::string&) {
    RuleExistsHelper::Instance()->registerRuleRegex( "[^ ]*pep_resource_resolve_hierarchy_pre" );
    return SUCCESS();
}

static
irods::error stop(irods::default_re_ctx&, const std::string&) {
    return SUCCESS();
}

static
irods::error rule_exists(irods::default_re_ctx&, const std::string& _rule_name, bool& _ret) {
    _ret = _rule_name == "pep_resource_resolve_hierarchy_pre";
    return SUCCESS();
}

static
irods::error list_rules( irods::default_re_ctx&, std::vector<std::string>& rule_vec ) {
    rule_vec.push_back("pep_resource_resolve_hierarchy_pre");
    return SUCCESS();
}

static
irods::error exec_rule(
    irods::default_re_ctx&,
    const std::string&     _rule_name,
    std::list<boost::any>& _rule_arguments,
    irods::callback        _effect_handler) {
    try {
        auto it_args{std::begin(_rule_arguments)};
        const auto& arg_resource_name{boost::any_cast<std::string&>(*it_args)};
        auto& arg_plugin_context{boost::any_cast<irods::plugin_context&>(*++it_args)};
        auto& arg_out{*boost::any_cast<std::string*>(*++it_args)};
        const auto& arg_operation_type{*boost::any_cast<const std::string*>(*++it_args)};
        const auto& arg_host{*boost::any_cast<const std::string*>(*++it_args)};
        auto& arg_hierarchy_parser{*boost::any_cast<irods::hierarchy_parser*>(*++it_args)};
        auto& arg_vote{*boost::any_cast<float*>(*++it_args)};

        if (arg_operation_type != "CREATE") {
            return SUCCESS();
        }

        ruleExecInfo_t& rei{get_rei(_effect_handler)};

        const std::string resource_type{get_resource_type(arg_resource_name, *rei.rsComm)};
        if (resource_type != "passthru") {
            return SUCCESS();
        }

        const boost::optional<uint64_t> max_bytes{get_max_bytes(arg_resource_name, *rei.rsComm)};
        if (!max_bytes) {
            arg_out = "read=1.0;write=1.0";
            return SUCCESS();
        } else if (*max_bytes == 0) {
            arg_out = "read=1.0;write=0.0";
            return SUCCESS();
        }
        const uint64_t bytes_used_by_children{get_bytes_used_by_all_children(arg_resource_name, *rei.rsComm)};
        const uint64_t bytes_required_for_new_data_object{get_bytes_of_incoming_data_object(arg_plugin_context)};
        const uint64_t hypothetical_bytes_used{bytes_used_by_children + bytes_required_for_new_data_object};
        const double percent_used{std::max(0.0, std::min(1.0, static_cast<double>(hypothetical_bytes_used) / *max_bytes))};
        const double write_weight{1.0 - percent_used};
        const std::string write_weight_string{boost::lexical_cast<std::string>(write_weight)};
        std::stringstream out_stream;
        out_stream << "read=1.0;write=" << write_weight_string;
        arg_out = out_stream.str();
        return SUCCESS();
    } catch (const irods::exception& e) {
        rodsLog(LOG_ERROR, e.what());
        return ERROR(e.code(), "irods exception in exec_rule");
    }
    return SUCCESS();
}

static
irods::error exec_rule_text(irods::default_re_ctx&, const std::string&, msParamArray_t*, const std::string&, irods::callback) {
    return ERROR(SYS_NOT_SUPPORTED, "not supported");
}

static
irods::error exec_rule_expression(irods::default_re_ctx&, const std::string&, msParamArray_t*, irods::callback) {
    return ERROR(SYS_NOT_SUPPORTED, "not supported");
}

extern "C"
irods::pluggable_rule_engine<irods::default_re_ctx>* plugin_factory(const std::string& _instance_name,
                                 const std::string& _context) {
    auto re{new irods::pluggable_rule_engine<irods::default_re_ctx>(_instance_name , _context)};
    re->add_operation(
            "start",
            std::function<irods::error(irods::default_re_ctx&, const std::string&)>(start));
    re->add_operation(
            "stop",
            std::function<irods::error(irods::default_re_ctx&, const std::string&)>(stop));
    re->add_operation(
            "rule_exists",
            std::function<irods::error(irods::default_re_ctx&, const std::string&, bool&)>(rule_exists));
    re->add_operation(
            "list_rules",
            std::function<irods::error(irods::default_re_ctx&,std::vector<std::string>&)>(list_rules));
    re->add_operation(
            "exec_rule",
            std::function<irods::error(irods::default_re_ctx&, const std::string&, std::list<boost::any>&, irods::callback)>(exec_rule));
    re->add_operation(
            "exec_rule_text",
            std::function<irods::error(irods::default_re_ctx&, const std::string&, msParamArray_t*, const std::string&, irods::callback)>(exec_rule_text));
    re->add_operation(
            "exec_rule_expression",
            std::function<irods::error(irods::default_re_ctx&, const std::string&, msParamArray_t*, irods::callback)>(exec_rule_expression));
    return re;
}
