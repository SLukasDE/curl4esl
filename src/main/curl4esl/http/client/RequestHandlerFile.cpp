/*
MIT License
Copyright (c) 2019, 2020 Sven Lukas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <curl4esl/http/client/RequestHandlerFile.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace curl4esl {
namespace http {
namespace client {

namespace {
std::size_t getFileSize(std::ifstream& file, const std::string& fileName) {
	if(!file.good()) {
        throw esl::addStacktrace(std::runtime_error("curl4esl: cannot open file \"" + fileName + "\""));
	}
	const auto begin = file.tellg();
	file.seekg(0, std::ios::end);
	const auto end = file.tellg();
	return (end-begin);
}
}

RequestHandlerFile::RequestHandlerFile(const esl::http::client::RequestFile& request)
: file(request.getFilename()),
  size(getFileSize(file, request.getFilename()))
{ }

size_t RequestHandlerFile::readDataCallback(void* data, size_t size, size_t nmemb, void* filePtr) {
	/* get upload data */
	RequestHandlerFile& requestHandler = *reinterpret_cast<RequestHandlerFile*>(filePtr);

    std::size_t remainingSize = requestHandler.size - requestHandler.pos;
    std::size_t count = size * nmemb;

    if(count > remainingSize) {
    	count = remainingSize;
    }
    requestHandler.pos += count;

    requestHandler.file.read(static_cast<char*>(data), count);

	return count;
}

const std::size_t RequestHandlerFile::getSize() const noexcept {
	return size;
}

} /* namespace client */
} /* namespace http */
} /* namespace curl4esl */
