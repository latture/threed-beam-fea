// Copyright 2015. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: ryan.latture@gmail.com (Ryan Latture)

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fea {
    /**
     * Takes a line from the input stream and appends it to the record.
     *
     * @param ins `std::istream`. The file where data is read from.
     * @param record `std::vector<T>`. Vector where the data is placed.
     */
    template<typename T>
    std::istream &operator>>(std::istream &ins, std::vector<T> &record) {
        // make sure that the returned record contains only the stuff we read now
        record.clear();

        // read the entire line into a string (a CSV record is terminated by a newline)
        std::string line;
        std::getline(ins, line);

        std::string empty;
        std::string separator_characters(", \t");
        boost::escaped_list_separator<char> separators(empty, separator_characters, empty);
        boost::tokenizer<boost::escaped_list_separator<char>> tk(line, separators);

        for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator i(tk.begin()); i != tk.end(); ++i) {
            T f = std::strtod(i->c_str(), 0);
            record.push_back(f);
        }
        return ins;
    }

    /**
     * Parses the file into a 2D vector. Each sub-vector is a line from the input file.
     *
     * @param ins `std::istream`. The file where data is read from. Each line will become a vector in `data`
     * @param data `std::vector< std::vector< T > >`. 2D Vector where the data is placed.
     */
    template<typename T>
    inline std::istream &operator>>(std::istream &ins, std::vector<std::vector<T> > &data) {
        // make sure that the returned data only contains the CSV data we read here
        data.clear();

        // For every record we can read from the file, append it to our resulting data
        std::vector<T> record;
        while (ins >> record) {
            data.push_back(record);
        }

        // Again, return the argument stream as required for this kind of input stream overload.
        return ins;
    }

    /**
     * @brief Reads data from a csv file into an `std::vector` and writes the contents of an `std::vector` to a file.
     * \note The parser assumes the data is comma delimited when reading data from a file.
     */
    class CSVParser {
    public:

        /**
         * @brief parses the contents of `file_name` into `data`.
         *
         * @param[in] file_name `std::string`. The file specified is opened and the contained information is parsed into `data`.
         * @param data `std::vector<T>`. Variable updated in place that will fold the data of the specified file.
         */
        template<typename T>
        void parseToVector(std::string filename, std::vector<T> &data) {

            std::ifstream infile;
            infile.open(filename);

            if (infile.is_open()) {
                try {
                    infile >> data;
                }
                catch (std::exception &e) {
                    throw std::runtime_error(
                            (boost::format("Error when parsing csv file %s.\nDetails from tokenizer:\n\t%s") %
                             filename % e.what()).str()
                    );
                }

                infile.close();
            }
            else {
                throw std::runtime_error(
                        (boost::format("Error opening file %s") % filename).str()
                );
            }
        }

        /**
         * Writes the 2D vector `data` to the file specified by `filename`.
         *
         * @param[in] filename `std::string`. The file to write data to.
         * @param[in] data `std::vector< std::vector< T > >`. Data to write to file.
         * @param[in] precision `unsigned int`. The number of decimal places to use when writing the data to file.
         * @param[in] demlimiter `std::string`. The delimiter to use between data entries.
         */
        template<typename T>
        void write(const std::string &filename,
                   const std::vector<std::vector<T> > &data,
                   unsigned int precision,
                   const std::string &delimiter) {
            std::ofstream output_file;
            output_file.open(filename);
            size_t num_cols;

            if (!output_file.is_open()) {
                throw std::runtime_error(
                        (boost::format("Error opening file %s") % filename).str()
                );
            }
            else {
                for (size_t i = 0; i < data.size(); ++i) {
                    num_cols = data[i].size();
                    for (size_t j = 0; j < num_cols; ++j) {
                        output_file << std::fixed << std::setprecision(precision) << data[i][j];
                        if (j < num_cols - 1) {
                            output_file << delimiter;
                        }
                    }
                    output_file << "\n";
                }
                output_file.close();
            }
        }
    };
} // namespace fea

#endif //CSV_PARSER_H
