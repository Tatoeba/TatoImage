/**
 * TatoImage  Web API to wrap basic image manipulation
 *
 * Copyright (C) 2014 Allan SIMON <allan.simon@supinfo.com>
 * See accompanying file COPYING.TXT file for licensing details.
 *
 * @category TatoImage
 * @author   Allan SIMON <allan.simon@supinfo.com> 
 * @package  Controllers
 * @link     https://github.com/allan-simon/tatoimage@
 *
 */
#include <curl/curl.h>

#include <cppcms/session_interface.h>
#include <cppcms/http_file.h>

#include <ImageMagick/Magick++.h>
#include "Images.h"


#include "contents/Images.h"

//%%%NEXT_INC_MODEL_CTRL_MARKER%%%


namespace tatoimage {
namespace controllers {
namespace webs {

Images::Images(cppcms::service& serv) :
    ::controllers::webs::Controller(serv)
{
    storageFolder = "../data/images/";
    originalFolder = storageFolder + "original/";

    dispatcher().assign("/resize", &Images::resize, this);

    dispatcher().assign("/upload-avatar", &Images::upload_avatar, this);
    dispatcher().assign("/upload-avatar_treat", &Images::upload_avatar_treat, this);


    dispatcher().assign("/normalize-avatar", &Images::normalize_avatar, this);
    //%%%NEXT_ACTION_DISPATCHER_MARKER%%%, do not delete


    //%%%NEXT_NEW_MODEL_CTRL_MARKER%%%
}

/**
 *
 */
Images::~Images() {
    //%%%NEXT_DEL_MODEL_CTRL_MARKER%%%
}

/**
 *
 */
void Images::upload_avatar() {

    forms::images::UploadAvatar form;
    contents::images::UploadAvatar c;
    init_content(c);


    render("images_upload_avatar", c);
}

/**
 *
 */
void Images::upload_avatar_treat() {

    forms::images::UploadAvatar form;
    form.load(context());

    if (!form.validate()) {
        go_back_to_previous_page();
        return;
    }

    std::istream& data = form.image.value()->data();

    //TODO should be possible to preallocate memory
    std::string imageBuffer((std::istreambuf_iterator<char>(data)), std::istreambuf_iterator<char>());
    //std::string imageBuffer(
    //    std::istreambuf_iterator<char>(data),
    //    std::istreambuf_iterator<char>()
    //);
    std::string imageName = form.filename.value();

    // we save to disk for persistence
    form.image.value()->save_to(
        originalFolder + imageName
    );

    cache().store_frame(
        imageName,
        imageBuffer
    );
}

/**
 * callback for curl writing data
 */
static size_t write_callback(
    void* contents,
    size_t size,
    size_t nmemb,
    void* userData
) {
    ((std::string*)userData)->append((char*)contents, size * nmemb);
    return size * nmemb;
};

/**
 *
 */
void Images::resize() {

    std::string filename = "";
    if (request().request_method() == "GET") {

        cppcms::http::request::form_type getData = request().get();
        cppcms::http::request::form_type::const_iterator it;

        GET_FIELD(filename, "filename");
    }

    std::string readBuffer;
    char* type = NULL;

    CURL* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, filename.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &readBuffer);


    CURLcode result = curl_easy_perform(handle);
    curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &type);

    response().content_type(type);
    response().out() << readBuffer;

    curl_easy_cleanup(handle);
}


/**
 *
 */
void Images::normalize_avatar() {

    std::string filename = "";
    if (request().request_method() == "GET") {

        cppcms::http::request::form_type getData = request().get();
        cppcms::http::request::form_type::const_iterator it;
        
        GET_FIELD(filename, "filename");
    }

    response().content_type("image/png");
    if (cache().fetch_page("normalize/" + filename)) {
        return;
    }

    // check if we have the file in cache
    std::string imageBuffer;
    if (!cache().fetch_frame(filename, imageBuffer)) {
        // if not in cache we test if we have the file on
        // disk
        std::ifstream file((originalFolder + filename).c_str());
        if(!file.good()) {
            response().status(404);
            response().out() << "404";
            return;
        }


        // we read the file and put original data in cache
        //TODO should be possible to preallocate memory
        imageBuffer.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        // TODO why this does not compile ?
        //imageBuffer.assign(
        //    (std::istreambuf_iterator<char>(file)),
        //    std::istreambuf_iterator<char>()
        //);
        file.close();

        cache().store_frame(
            filename,
            imageBuffer
        );
    }

    // we first load our image from its string representation
    // to an Magick++ Image
    Magick::Blob initialBlob(imageBuffer.data(), imageBuffer.length());
    Magick::Image workingImage(initialBlob);

    //we prepare a transparent PNG image of 50*50 pixels
    Magick::Image blankImage(
        Magick::Geometry(50, 50),
        Magick::Color(
            0,
            0,
            0,
            (Magick::Quantum) 65535
        )
    );
    blankImage.magick("PNG");

    //we resize the actual image to 50 for largest dimension
    //while keeping ratio
    workingImage.resize(Magick::Geometry(50,50));
    //and we paste it at top left of the "transparent" image
    blankImage.composite(workingImage, 0, 0, Magick::OverCompositeOp);
    blankImage.write(&initialBlob);

    std::string finalImage(
        static_cast<const char*>(initialBlob.data()),
        initialBlob.length()
    );

    response().out() << finalImage;

    cache().store_page("normalize/" + filename);

}

// %%%NEXT_ACTION_MARKER%%% , do not delete




} // End namespace webs
} // End namespace controllers
} // End namespace tatoimage

