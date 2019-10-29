#pragma once

#include "driver/driver.h"
#include "driver/connection.h"
#include "driver/descriptor.h"
#include "driver/result_set.h"

#include <Poco/Net/HTTPResponse.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Statement
    : public Child<Connection, Statement>
{
private:
    using ChildType = Child<Connection, Statement>;

public:
    explicit Statement(Connection & connection);
    virtual ~Statement();

    /// Lookup TypeInfo for given name of type.
    const TypeInfo & getTypeInfo(const std::string & type_name, const std::string & type_name_without_parametrs = "") const;

    /// Prepare query for execution.
    void prepareQuery(const std::string & q);

    /// Execute previously prepared query.
    void executeQuery(IResultMutatorPtr && mutator = IResultMutatorPtr{});

    /// Prepare and execute query.
    void executeQuery(const std::string & q, IResultMutatorPtr && mutator = IResultMutatorPtr {});

    /// Indicates whether there is an result set available for reading.
    bool hasResultSet() const;

    /// Make the next result set current, if any.
    bool advanceToNextResultSet();

    const ColumnInfo & getColumnInfo(size_t i) const;

    size_t getNumColumns() const;

    bool hasCurrentRow() const;

    const Row & getCurrentRow() const;

    /// Checked way of retrieving the number of the current row in the current result set.
    std::size_t getCurrentRowNum() const;

    bool advanceToNextRow();

    /// Reset statement to initial state.
    void closeCursor();

    /// Reset/release row/column buffer bindings.
    void resetColBindings();

    /// Reset/release parameter buffer bindings.
    void resetParamBindings();

    /// Access the effective descriptor by its role (type).
    Descriptor & getEffectiveDescriptor(SQLINTEGER type);

    /// Set an explicit descriptor for a role (type).
    void setExplicitDescriptor(SQLINTEGER type, std::shared_ptr<Descriptor> desc);

    /// Make an implicit descriptor active again.
    void setImplicitDescriptor(SQLINTEGER type);

private:
    void requestNextPackOfResultSets(IResultMutatorPtr && mutator);

    void processEscapeSequences();
    void extractParametersinfo();
    std::string buildFinalQuery(const std::vector<ParamBindingInfo>& param_bindings);
    std::string getParamFinalName(std::size_t param_idx);
    std::vector<ParamBindingInfo> getParamsBindingInfo(std::size_t param_set_idx);

    Descriptor & choose(std::shared_ptr<Descriptor> & implicit_desc, std::weak_ptr<Descriptor> & explicit_desc);

    void allocateImplicitDescriptors();
    void deallocateImplicitDescriptors();

    std::shared_ptr<Descriptor> allocateDescriptor();
    void deallocateDescriptor(std::shared_ptr<Descriptor> & desc);

private:
    std::shared_ptr<Descriptor> implicit_ard;
    std::shared_ptr<Descriptor> implicit_apd;
    std::shared_ptr<Descriptor> implicit_ird;
    std::shared_ptr<Descriptor> implicit_ipd;

    std::weak_ptr<Descriptor> explicit_ard;
    std::weak_ptr<Descriptor> explicit_apd;
    std::weak_ptr<Descriptor> explicit_ird;
    std::weak_ptr<Descriptor> explicit_ipd;

    std::string query;
    std::vector<ParamInfo> parameters;

    std::unique_ptr<Poco::Net::HTTPResponse> response;
    std::istream* in = nullptr;
    std::unique_ptr<ResultSet> result_set;
    std::size_t next_param_set = 0;

public:
    // TODO: switch to using the corresponding descriptor attributes.
    std::map<SQLUSMALLINT, BindingInfo> bindings;
};
