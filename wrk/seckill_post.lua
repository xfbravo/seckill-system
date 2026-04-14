wrk.method = "POST"                                                                           
wrk.headers["Content-Type"] = "application/json"
                                                                                            
request = function()                                      
    local user_id = math.random(10000, 99999)
    local body = '{"activity_id":12,"user_id":' .. user_id .. ',"quantity":1}'                 
    return wrk.format(nil, nil, nil, body)
end