{
	"service" : {
		"api" : "http",
		"port" : 8080
	},

    "cache" : {
        "backend" : "thread_shared"
    },
	"http" : {
		"script_names" : ["/TatoImage"]
	},
	"localization" : {
		"encoding" : "utf-8",
		"messages" : {
			"paths" : [ "../locale" ],
			"domains" : [ "hello" ]
		},
		"locales" : [ "en_GB.UTF-8", "fr_FR.UTF-8" ]
	},
    "session" : {
        "expire" : "renew",
        "timeout" : 604800,
        "location" : "server",
        "server" : {
            "storage" : "memory"
        }
    },
    "skel" : {
        "baseHost" : "",
        "assetsFolder" : "../resources/",
        "uploadFolder" : "../upload/",
        "assetsURL" : "/",
        "uploadURL" : "/upload/"
    },
	"TatoImage" : {
        "interfacelangs" : [
            ["en"  , "en_GB.UTF-8", "English", "English"],
            ["fr"  , "fr_FR.UTF-8", "Français", "French"]
        ],
        "sqlite3" : {
            "path" : "../data/sqlite3.db"
        }
	},
   "views" : {
        "default_skin" : "default_skin"
    }


}
