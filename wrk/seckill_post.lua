wrk.method = "POST"                                                                           
wrk.headers["Content-Type"] = "application/json"
                                                                                            
request = function()                                      
    local user_id = math.random(100000, 999999)
    local body = '{"activity_id":15,"user_id":' .. user_id .. ',"quantity":1}'                 
    return wrk.format(nil, nil, nil, body)
end