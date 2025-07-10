-- Cabin Framework Library

target("cabin")
    set_kind("static")
    add_includedirs(".", { public = true })
    add_files(
        "cabin/*.cc",
        "cabin/core/*.cc"
    )