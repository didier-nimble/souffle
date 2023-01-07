#pragma once


#include "souffle/RamTypes.h"
#include "souffle/RecordTable.h"
#include "souffle/SymbolTable.h"
#include "souffle/io/ReadStream.h"
#include "souffle/utility/MiscUtil.h"
#include "souffle/utility/StringUtil.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <duckdb.h>

namespace souffle {

class ReadStreamDuckDB:  public ReadStream  {
public:
    ReadStreamDuckDB(const std::map<std::string, std::string>& rwOperation, 
                     SymbolTable& symbolTable,
                     RecordTable& recordTable):  ReadStream(rwOperation, symbolTable, recordTable),
                                                 dbFilename(getFileName(rwOperation)),
                                                 relationName(rwOperation.at("name"))
                                                {
        openDB();

    }

    ~ReadStreamDuckDB() override {
        
    }

protected:

    void openDB(){
        // duckdb_database db;
        duckdb_create_config(&config);
        duckdb_open_ext(NULL, &db, config, NULL);
        // duckdb::DBConfig config;
        // db = duckdb::DuckDB(dbFilename, &config);

    }


    /**
     * Return given filename or construct from relation name.
     * Default name is [configured path]/[relation name].sqlite
     *
     * @param rwOperation map of IO configuration options
     * @return input filename
     */
    static std::string getFileName(const std::map<std::string, std::string>& rwOperation) {
        auto name = rwOperation.at("filename");
        if (name.front() != '/') {
            name = getOr(rwOperation, "fact-dir", ".") + "/" + name;
        }
        return name;
    }
    duckdb_config config;
    duckdb_database db;
    const std::string dbFilename;
    const std::string relationName;
    // duckdb::DuckDB db;
};
}