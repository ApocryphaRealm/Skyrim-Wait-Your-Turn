rule("template.copy_resource")
    on_install(function(target)
        local conf = target:extraconf("rules", "template.copy_resource")
        local config_src = path.join(os.projectdir(), conf.src)
        local config_dst = path.join(target:installdir(), conf.dest)
        if os.isdir(config_src) then
            os.tryrm(config_dst)
            os.mkdir(config_dst)
            os.trycp(path.join(config_src, "**"), config_dst)
        elseif os.isfile(config_src) then 
            os.mkdir(path.directory(config_src))
            os.tryrm(config_dst)
            os.trycp(config_src, config_dst)
        else
            print("copy_resource: src not found at ", config_src)
        end
    end)