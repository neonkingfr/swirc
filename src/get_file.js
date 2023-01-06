function get_file(url, name)
{
    var http_req = new ActiveXObject("WinHttp.WinHttpRequest.5.1");
    var stream   = new ActiveXObject("ADODB.Stream");

    http_req.Open("GET", url, false);
    http_req.Send();
    http_req.WaitForResponse();

    stream.Type = 1;
    stream.Open();
    stream.Write(http_req.ResponseBody);
    stream.SaveToFile(name);
}

get_file("https://curl.haxx.se/ca/cacert.pem", "src/trusted_roots.pem");
get_file("https://www.nifty-networks.net/swirc/curl-7.83.0.cab", "curl-7.83.0.cab");
get_file("https://www.nifty-networks.net/swirc/gnu-bundle-202205.cab", "gnu-bundle-202205.cab");
get_file("https://www.nifty-networks.net/swirc/libressl-3.6.1-windows.cab", "libressl-3.6.1-windows.cab");
get_file("https://www.nifty-networks.net/swirc/pdcurses-3.9.cab", "pdcurses-3.9.cab");
get_file("https://www.nifty-networks.net/swirc/swirc-locales-20230106.cab", "swirc-locales-20230106.cab");
