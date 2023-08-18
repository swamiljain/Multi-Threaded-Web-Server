#include "http_server.hh"

#include <vector>

#include <sys/stat.h>

#include <fstream>
#include <sstream>

vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request) {
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request


  /*
   TODO : extract the request method and URL from first_line here
  */

  this->method=first_line[0];
  this->url=first_line[1];
  if (this->method != "GET") {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req) {
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;
  response->HTTP_version = "1.0";

  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body="";

    if (S_ISDIR(sb.st_mode)) {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
     string tmp_url=url + string("/index.html");
     if(stat(tmp_url.c_str(), &sb) == 0)
      {
        url=url + string("/index.html");

        /*
    TODO : open the file and read its contents
    */
      ifstream ip_file;
      ip_file.open(url);
      string temp_line;
      while(getline(ip_file,temp_line))
          {
            temp_line=temp_line + string("\n");
            body.append(temp_line);
          }
   
    /*
    TODO : set the remaining fields of response appropriately
    */
    stringstream inttostr;
    inttostr<<body.length(); 
    inttostr>>response->content_length;
    response->body=body;
      }
     else
     {
        response->status_code = "404";
        response->status_text = "Not Found";
        response->content_type = "text/html";
        string body="<html>\n<body>\n<h1><font color='red'><center>404 : Requested Page Doesn't Exist</center></font></h1></body>\n</html>\n";

        stringstream inttostr;
        inttostr<<body.length(); 
        inttostr>>response->content_length;
        response->body=body;
     }

    }
    
    else
  { 
    /*
    TODO : open the file and read its contents
    */
      ifstream ip_file;
      ip_file.open(url);
      string temp_line;
      while(getline(ip_file,temp_line))
          {
            temp_line=temp_line + string("\n");
            body.append(temp_line);
          }
   
    /*
    TODO : set the remaining fields of response appropriately
    */
    stringstream inttostr;
    inttostr<<body.length(); 
    inttostr>>response->content_length;
    response->body=body;
  }
  }

  else {
    response->status_code = "404";
    response->status_text = "Not Found";
    response->content_type = "text/html";
    string body="<html>\n<body>\n<h1><font color='red'><center>404 : Requested Page Doesn't Exist</center></font></h1></body>\n</html>\n";

    stringstream inttostr;
    inttostr<<body.length();
    inttostr>>response->content_length;
    response->body=body;
    /*
    TODO : set the remaining fields of response appropriately
    */

  }

  delete request;

  return response;
}

string HTTP_Response::get_string() {
  /*
  TODO : implement this function
  */
 string all_contents="";
 string space=" ";
 all_contents = all_contents + string("HTTP/") + HTTP_version + space + status_code + space + status_text + string("\n");
 all_contents = all_contents + string("Content-Type: ") + content_type + string("\n");
 all_contents = all_contents + string("Content-Length: ") + content_length + string("\n\n");
 all_contents = all_contents + body;
 
 
 return all_contents;
}