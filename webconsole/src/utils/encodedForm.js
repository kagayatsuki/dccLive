function encodeObj2Form(obj) {
    let tmp = []
    for (let key in obj)
        tmp.push(encodeURIComponent(key) + '=' + encodeURIComponent(obj[key]))
    return tmp.join('&')
}

export default encodeObj2Form