
#include "../include/PostMethod.hpp"

std::string urlDecode(const std::string& encoded_str) {
    std::string decoded;
    decoded.reserve(encoded_str.length()); 

    for (size_t i = 0; i < encoded_str.length(); ++i) {
        if (encoded_str[i] == '+') {
            decoded += ' ';
        } else if (encoded_str[i] == '%' && i + 2 < encoded_str.length()) {
            std::string hex_str = encoded_str.substr(i + 1, 2);
            char decoded_char = static_cast<char>(std::strtol(hex_str.c_str(), NULL, 16));
            decoded += decoded_char;
            i += 2;
        } else {
            decoded += encoded_str[i];
        }
    }
    return decoded;
}

std::map<std::string, std::string> PostMethod::parseUrlEncodedForm(const std::string& body) {
    std::map<std::string, std::string> form_data;
    std::istringstream stream(body);
    std::string pair;

    while (std::getline(stream, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eq_pos));
            std::string value = urlDecode(pair.substr(eq_pos + 1));
            form_data[key] = value;
        } else if (!pair.empty())
            form_data[urlDecode(pair)] = "";
    }
    return form_data;
}

std::vector<MultiPartData> PostMethod::parseMultipartBody(const std::string& body, const std::string& boundary) {
    std::vector<MultiPartData> parts;
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = delimiter + "--";

    size_t pos = body.find(delimiter);
    while (pos != std::string::npos) {
        size_t start_of_part = body.find("\r\n", pos);
        if (start_of_part == std::string::npos) break;
        start_of_part += 2;
        size_t next_boundary_pos = body.find(delimiter, start_of_part);
        if (next_boundary_pos == std::string::npos)
            break;
        size_t end_of_part = next_boundary_pos - 2; 
        std::string raw_part = body.substr(start_of_part, end_of_part - start_of_part);
        size_t header_end = raw_part.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            MultiPartData part;
            part.is_file = false;
            std::string headers_str = raw_part.substr(0, header_end);
            part.data = raw_part.substr(header_end + 4);
            std::istringstream header_stream(headers_str);
            std::string header_line;
            while (std::getline(header_stream, header_line)) {
                if (!header_line.empty() && header_line[header_line.size() - 1] == '\r') {
                    header_line.erase(header_line.size() - 1);
                }
                size_t colon_pos = header_line.find(": ");
                if (colon_pos != std::string::npos) {
                    std::string h_key = header_line.substr(0, colon_pos);
                    std::string h_val = header_line.substr(colon_pos + 2);
                    part.headers[h_key] = h_val;
                    if (h_key == "Content-Disposition") {
                        size_t name_pos = h_val.find("name=\"");
                        if (name_pos != std::string::npos) {
                            name_pos += 6;
                            size_t name_end = h_val.find("\"", name_pos);
                            part.name = h_val.substr(name_pos, name_end - name_pos);
                        }
                        size_t file_pos = h_val.find("filename=\"");
                        if (file_pos != std::string::npos) {
                            file_pos += 10;
                            size_t file_end = h_val.find("\"", file_pos);
                            part.filename = h_val.substr(file_pos, file_end - file_pos);
                            part.is_file = true;
                        }
                    }
                }
            }
            parts.push_back(part);
        }

        pos = next_boundary_pos;
        if (body.compare(pos, end_delimiter.length(), end_delimiter) == 0) {
            break;
        }
    }
    return parts;
}


void PostMethod::execute(const HttpRequest& req, HttpResponse& res, const LocationConfig& config)
{
    std::string contentType = req.getHeader("Content-Type");
    if(contentType.find("multipart/form-data") != std::string::npos)
    {
        size_t pos = contentType.find("boundary=") + 9;
        std::string boundary = contentType.substr(pos);
        std::vector<MultiPartData> parsedParts = parseMultipartBody(req.getBody(), boundary);
        std::string response_info = "Upload Summary:\n";
        for (size_t i = 0; i < parsedParts.size(); ++i) {
            if (parsedParts[i].is_file) {
                std::string full_path = config.upload_path + config.path + "/" + parsedParts[i].filename;
                std::cerr << "Saving uploaded file to: " << full_path << std::endl;
                std::ofstream outfile(full_path.c_str(), std::ios::binary | std::ios::trunc);
                if (outfile.is_open()) {
                    outfile << parsedParts[i].data;
                    outfile.close();
                    response_info += "Saved file: " + parsedParts[i].filename + "\n";
                }else
                {
                    res.setStatusCode(403);
                    res.setErrorBody("403 Forbidden: Permission denied");
                    return;
                }
            } else {
                response_info += "Form field [" + parsedParts[i].name + "] = " + parsedParts[i].data + "\n";
            }
        }
        res.setStatusCode(201);
        res.setBody(response_info);
        res.setHeader("Content-Type", "text/plain");
        return;
    }
    std::string full_path = config.upload_path + req.getPath();
    std::cerr << "Saving uploaded data to: " << full_path << std::endl;
    std::ofstream outfile(full_path.c_str(), std::ios::binary | std::ios::trunc);
    if(!outfile.is_open())
    {
        res.setStatusCode(403);
        res.setErrorBody("403 Forbidden: Permission denied");
        return;
    }
    
    if(contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::map<std::string, std::string> formData = parseUrlEncodedForm(req.getBody());
        outfile << "Parsed Form Data:\n";
        for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it) {
            outfile << it->first << ": " << it->second << "\n";
        }
    }
    else
    {
        outfile << req.getBody();
    }
    outfile.close();
    res.setStatusCode(201);
    res.setBody("201 File uploaded successfully");
    res.setHeader("Content-Type", "text/plain");
    // 1. Determine upload directory from config
    // 2. Extract body from req
    // 3. Generate unique filename or use provided filename
    // 4. Write body to file in upload directory
    // 5. Set appropriate response status and headers
}