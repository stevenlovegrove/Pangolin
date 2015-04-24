/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Robert Castle, Gabe Sibley, Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string>

namespace pangolin {

std::string font_xml_data = "<?xml version=\"1.0\"?>\n"
    "<font>\n"
    "  <info face=\"Courier New\" size=\"16\" bold=\"0\" italic=\"0\" charset=\"ANSI\" unicode=\"0\" stretchH=\"100\" smooth=\"0\" aa=\"1\" padding=\"2,1,1,1\" spacing=\"1,2\" outline=\"0\"/>\n"
    "  <common lineHeight=\"16\" base=\"12\" scaleW=\"256\" scaleH=\"128\" pages=\"1\" packed=\"0\" alphaChnl=\"0\" redChnl=\"0\" greenChnl=\"0\" blueChnl=\"0\"/>\n"
    "  <chars count=\"218\">\n"
    "    <char id=\"32\" x=\"111\" y=\"92\" width=\"3\" height=\"4\" xoffset=\"-1\" yoffset=\"10\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"33\" x=\"249\" y=\"42\" width=\"3\" height=\"11\" xoffset=\"3\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"34\" x=\"118\" y=\"83\" width=\"7\" height=\"7\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"35\" x=\"59\" y=\"16\" width=\"8\" height=\"13\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"36\" x=\"100\" y=\"16\" width=\"6\" height=\"13\" xoffset=\"1\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"37\" x=\"56\" y=\"71\" width=\"7\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"38\" x=\"122\" y=\"71\" width=\"7\" height=\"10\" xoffset=\"1\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"39\" x=\"198\" y=\"80\" width=\"3\" height=\"7\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"40\" x=\"119\" y=\"16\" width=\"4\" height=\"13\" xoffset=\"3\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"41\" x=\"124\" y=\"16\" width=\"4\" height=\"13\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"42\" x=\"44\" y=\"84\" width=\"7\" height=\"9\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"43\" x=\"68\" y=\"71\" width=\"9\" height=\"10\" xoffset=\"0\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"44\" x=\"180\" y=\"80\" width=\"5\" height=\"7\" xoffset=\"1\" yoffset=\"8\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"45\" x=\"87\" y=\"93\" width=\"8\" height=\"4\" xoffset=\"0\" yoffset=\"6\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"46\" x=\"26\" y=\"96\" width=\"4\" height=\"5\" xoffset=\"2\" yoffset=\"8\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"47\" x=\"248\" y=\"0\" width=\"7\" height=\"13\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"48\" x=\"108\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"49\" x=\"117\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"50\" x=\"48\" y=\"71\" width=\"7\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"51\" x=\"24\" y=\"71\" width=\"7\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"52\" x=\"126\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"53\" x=\"135\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"54\" x=\"40\" y=\"71\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"55\" x=\"153\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"56\" x=\"162\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"57\" x=\"189\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"58\" x=\"65\" y=\"84\" width=\"4\" height=\"9\" xoffset=\"2\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"59\" x=\"130\" y=\"71\" width=\"5\" height=\"10\" xoffset=\"1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"60\" x=\"105\" y=\"71\" width=\"8\" height=\"10\" xoffset=\"0\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"61\" x=\"202\" y=\"80\" width=\"8\" height=\"6\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"62\" x=\"96\" y=\"71\" width=\"8\" height=\"10\" xoffset=\"0\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"63\" x=\"32\" y=\"71\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"64\" x=\"76\" y=\"16\" width=\"7\" height=\"13\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"65\" x=\"150\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"66\" x=\"198\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"67\" x=\"240\" y=\"42\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"68\" x=\"0\" y=\"59\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"69\" x=\"9\" y=\"59\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"70\" x=\"81\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"71\" x=\"90\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"72\" x=\"80\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"73\" x=\"16\" y=\"72\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"74\" x=\"18\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"75\" x=\"50\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"76\" x=\"36\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"77\" x=\"40\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"78\" x=\"212\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"79\" x=\"10\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"80\" x=\"45\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"81\" x=\"227\" y=\"15\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"82\" x=\"230\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"83\" x=\"63\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"84\" x=\"140\" y=\"44\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"85\" x=\"0\" y=\"46\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"86\" x=\"223\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"87\" x=\"30\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"88\" x=\"245\" y=\"29\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"89\" x=\"20\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"90\" x=\"232\" y=\"56\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"91\" x=\"113\" y=\"16\" width=\"5\" height=\"13\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"92\" x=\"84\" y=\"16\" width=\"7\" height=\"13\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"93\" x=\"107\" y=\"16\" width=\"5\" height=\"13\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"94\" x=\"110\" y=\"83\" width=\"7\" height=\"7\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"95\" x=\"36\" y=\"95\" width=\"10\" height=\"4\" xoffset=\"-1\" yoffset=\"13\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"96\" x=\"16\" y=\"96\" width=\"4\" height=\"5\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"97\" x=\"180\" y=\"69\" width=\"9\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"98\" x=\"160\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"99\" x=\"0\" y=\"85\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"100\" x=\"130\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"101\" x=\"9\" y=\"85\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"102\" x=\"72\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"103\" x=\"100\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"104\" x=\"220\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"105\" x=\"216\" y=\"56\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"106\" x=\"92\" y=\"16\" width=\"7\" height=\"13\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"107\" x=\"200\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"108\" x=\"224\" y=\"56\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"109\" x=\"169\" y=\"69\" width=\"10\" height=\"9\" xoffset=\"-1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"110\" x=\"200\" y=\"69\" width=\"9\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"111\" x=\"18\" y=\"85\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"112\" x=\"99\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"113\" x=\"120\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"114\" x=\"229\" y=\"69\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"115\" x=\"27\" y=\"84\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"116\" x=\"78\" y=\"71\" width=\"8\" height=\"10\" xoffset=\"0\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"117\" x=\"210\" y=\"69\" width=\"9\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"118\" x=\"147\" y=\"70\" width=\"10\" height=\"9\" xoffset=\"-1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"119\" x=\"190\" y=\"69\" width=\"9\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"120\" x=\"247\" y=\"68\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"121\" x=\"180\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"122\" x=\"36\" y=\"84\" width=\"7\" height=\"9\" xoffset=\"1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"123\" x=\"129\" y=\"31\" width=\"5\" height=\"12\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"124\" x=\"129\" y=\"16\" width=\"3\" height=\"13\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"125\" x=\"123\" y=\"31\" width=\"5\" height=\"12\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"126\" x=\"0\" y=\"96\" width=\"8\" height=\"5\" xoffset=\"0\" yoffset=\"5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"128\" x=\"60\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"130\" x=\"186\" y=\"80\" width=\"5\" height=\"7\" xoffset=\"1\" yoffset=\"8\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"131\" x=\"207\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"132\" x=\"220\" y=\"80\" width=\"8\" height=\"6\" xoffset=\"0\" yoffset=\"8\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"133\" x=\"96\" y=\"92\" width=\"7\" height=\"4\" xoffset=\"0\" yoffset=\"9\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"134\" x=\"0\" y=\"72\" width=\"7\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"135\" x=\"8\" y=\"72\" width=\"7\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"136\" x=\"250\" y=\"79\" width=\"5\" height=\"5\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"137\" x=\"168\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"138\" x=\"150\" y=\"0\" width=\"8\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"139\" x=\"52\" y=\"84\" width=\"6\" height=\"9\" xoffset=\"-1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"140\" x=\"157\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"142\" x=\"202\" y=\"0\" width=\"7\" height=\"14\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"145\" x=\"168\" y=\"80\" width=\"5\" height=\"7\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"146\" x=\"174\" y=\"80\" width=\"5\" height=\"7\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"147\" x=\"211\" y=\"80\" width=\"8\" height=\"6\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"148\" x=\"229\" y=\"80\" width=\"8\" height=\"6\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"149\" x=\"238\" y=\"80\" width=\"5\" height=\"6\" xoffset=\"2\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"150\" x=\"78\" y=\"93\" width=\"8\" height=\"4\" xoffset=\"0\" yoffset=\"6\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"151\" x=\"47\" y=\"95\" width=\"10\" height=\"4\" xoffset=\"-1\" yoffset=\"6\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"152\" x=\"9\" y=\"96\" width=\"6\" height=\"5\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"153\" x=\"89\" y=\"83\" width=\"10\" height=\"7\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"154\" x=\"247\" y=\"15\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"155\" x=\"59\" y=\"84\" width=\"5\" height=\"9\" xoffset=\"3\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"156\" x=\"136\" y=\"71\" width=\"10\" height=\"9\" xoffset=\"-1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"158\" x=\"107\" y=\"31\" width=\"7\" height=\"12\" xoffset=\"1\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"159\" x=\"20\" y=\"16\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"160\" x=\"115\" y=\"92\" width=\"3\" height=\"4\" xoffset=\"-1\" yoffset=\"10\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"161\" x=\"64\" y=\"71\" width=\"3\" height=\"11\" xoffset=\"3\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"162\" x=\"114\" y=\"71\" width=\"7\" height=\"10\" xoffset=\"1\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"163\" x=\"54\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"164\" x=\"238\" y=\"69\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"165\" x=\"70\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"166\" x=\"133\" y=\"16\" width=\"3\" height=\"13\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"167\" x=\"28\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"168\" x=\"104\" y=\"92\" width=\"6\" height=\"4\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"169\" x=\"201\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"170\" x=\"147\" y=\"81\" width=\"6\" height=\"7\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"171\" x=\"70\" y=\"83\" width=\"9\" height=\"8\" xoffset=\"0\" yoffset=\"5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"172\" x=\"100\" y=\"83\" width=\"9\" height=\"7\" xoffset=\"-1\" yoffset=\"6\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"173\" x=\"69\" y=\"95\" width=\"8\" height=\"4\" xoffset=\"0\" yoffset=\"6\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"174\" x=\"179\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"175\" x=\"58\" y=\"95\" width=\"10\" height=\"4\" xoffset=\"-1\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"176\" x=\"154\" y=\"81\" width=\"6\" height=\"7\" xoffset=\"1\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"177\" x=\"110\" y=\"45\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"178\" x=\"140\" y=\"82\" width=\"6\" height=\"7\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"179\" x=\"133\" y=\"83\" width=\"6\" height=\"7\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"180\" x=\"21\" y=\"96\" width=\"4\" height=\"5\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"181\" x=\"190\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"-1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"182\" x=\"157\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"183\" x=\"31\" y=\"95\" width=\"4\" height=\"5\" xoffset=\"2\" yoffset=\"5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"184\" x=\"244\" y=\"80\" width=\"5\" height=\"6\" xoffset=\"2\" yoffset=\"9\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"185\" x=\"192\" y=\"80\" width=\"5\" height=\"7\" xoffset=\"2\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"186\" x=\"161\" y=\"80\" width=\"6\" height=\"7\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"187\" x=\"80\" y=\"83\" width=\"8\" height=\"8\" xoffset=\"0\" yoffset=\"5\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"188\" x=\"234\" y=\"29\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"189\" x=\"135\" y=\"31\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"190\" x=\"146\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"191\" x=\"240\" y=\"55\" width=\"7\" height=\"11\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"192\" x=\"31\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"193\" x=\"21\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"194\" x=\"131\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"195\" x=\"121\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"196\" x=\"0\" y=\"17\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"197\" x=\"0\" y=\"0\" width=\"9\" height=\"15\" xoffset=\"0\" yoffset=\"-2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"198\" x=\"190\" y=\"30\" width=\"10\" height=\"11\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"199\" x=\"168\" y=\"0\" width=\"8\" height=\"14\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"200\" x=\"177\" y=\"0\" width=\"8\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"201\" x=\"141\" y=\"0\" width=\"8\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"202\" x=\"159\" y=\"0\" width=\"8\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"203\" x=\"50\" y=\"16\" width=\"8\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"204\" x=\"210\" y=\"0\" width=\"7\" height=\"14\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"205\" x=\"186\" y=\"0\" width=\"7\" height=\"14\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"206\" x=\"194\" y=\"0\" width=\"7\" height=\"14\" xoffset=\"1\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"207\" x=\"68\" y=\"16\" width=\"7\" height=\"13\" xoffset=\"1\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"208\" x=\"27\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"209\" x=\"10\" y=\"0\" width=\"10\" height=\"14\" xoffset=\"-1\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"210\" x=\"30\" y=\"16\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"211\" x=\"111\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"212\" x=\"101\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"213\" x=\"91\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"214\" x=\"40\" y=\"16\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"215\" x=\"126\" y=\"83\" width=\"6\" height=\"7\" xoffset=\"1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"216\" x=\"217\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"217\" x=\"81\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"218\" x=\"71\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"219\" x=\"61\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"220\" x=\"218\" y=\"0\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"221\" x=\"51\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"-1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"222\" x=\"171\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"223\" x=\"180\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"224\" x=\"197\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"225\" x=\"187\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"226\" x=\"177\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"227\" x=\"167\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"228\" x=\"170\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"229\" x=\"238\" y=\"0\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"0\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"230\" x=\"158\" y=\"69\" width=\"10\" height=\"9\" xoffset=\"-1\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"231\" x=\"55\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"232\" x=\"82\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"233\" x=\"73\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"234\" x=\"64\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"235\" x=\"207\" y=\"56\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"236\" x=\"115\" y=\"31\" width=\"7\" height=\"12\" xoffset=\"1\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"237\" x=\"99\" y=\"31\" width=\"7\" height=\"12\" xoffset=\"1\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"238\" x=\"91\" y=\"31\" width=\"7\" height=\"12\" xoffset=\"1\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"239\" x=\"248\" y=\"55\" width=\"7\" height=\"11\" xoffset=\"1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"240\" x=\"90\" y=\"58\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"241\" x=\"147\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"242\" x=\"10\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"243\" x=\"46\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"244\" x=\"37\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"245\" x=\"19\" y=\"31\" width=\"8\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"246\" x=\"144\" y=\"57\" width=\"8\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"247\" x=\"87\" y=\"71\" width=\"8\" height=\"10\" xoffset=\"0\" yoffset=\"3\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"248\" x=\"220\" y=\"69\" width=\"8\" height=\"9\" xoffset=\"0\" yoffset=\"4\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"249\" x=\"137\" y=\"16\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"250\" x=\"237\" y=\"15\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"251\" x=\"0\" y=\"32\" width=\"9\" height=\"12\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"252\" x=\"210\" y=\"43\" width=\"9\" height=\"11\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"253\" x=\"41\" y=\"0\" width=\"9\" height=\"14\" xoffset=\"0\" yoffset=\"1\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"254\" x=\"10\" y=\"16\" width=\"9\" height=\"13\" xoffset=\"-1\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "    <char id=\"255\" x=\"228\" y=\"0\" width=\"9\" height=\"13\" xoffset=\"0\" yoffset=\"2\" xadvance=\"8\" page=\"0\" chnl=\"15\" />\n"
    "  </chars>\n"
    "</font>\n"
    "\n"
    "";

// The embedded font image. this is a 256x128x1 image
unsigned char font_image_data[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
  0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,255,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,0,255,0,
  0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,
  0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,
  255,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,255,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,255,0,
  0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,
  0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,0,255,0,0,
  0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,255,255,255,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,
  255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,0,0,
  0,0,0,0,0,0,255,255,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,255,255,255,0,255,0,0,0,255,255,255,255,255,255,0,0,0,255,0,0,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,255,255,
  0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,
  0,0,0,255,255,0,0,0,0,0,0,0,255,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,
  0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,255,255,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,
  0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,
  0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,
  0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,0,
  0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,0,255,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,
  0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,0,0,
  0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,
  0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,0,
  0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,255,255,0,
  0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,
  0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,255,0,0,0,0,255,0,0,0,0,0,0,
  0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,
  0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,
  0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,255,255,0,0,0,255,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,255,255,0,0,255,255,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,
  255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,255,255,
  255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,255,255,255,0,0,0,255,0,255,255,255,0,0,0,0,255,255,255,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,255,255,
  0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,255,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,
  0,255,0,0,0,255,0,0,255,0,0,0,0,0,255,255,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,255,255,255,0,0,0,255,255,255,0,0,0,0,255,0,0,0,255,0,0,
  0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,0,255,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,255,255,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,
  255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,
  0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,
  0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,
  0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,255,0,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,255,0,255,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,255,
  0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,255,0,255,255,255,0,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,255,0,0,
  255,0,0,0,0,255,255,255,255,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,255,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,
  0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,255,0,0,255,255,0,0,0,0,0,255,255,255,255,255,0,0,
  0,0,0,255,255,0,0,0,0,0,0,0,255,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,255,255,255,
  255,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,255,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,
  0,0,255,0,0,0,255,0,0,0,255,255,0,0,255,255,0,0,0,0,255,255,0,255,255,0,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,255,255,
  255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,255,0,0,255,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,
  0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,
  255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,
  0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,0,255,0,0,0,0,0,255,255,255,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,
  0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,
  255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,
  0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,255,255,
  255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,255,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,255,255,255,255,
  255,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,
  0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,255,255,0,0,0,0,255,0,0,0,0,255,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,0,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,
  255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,255,0,0,0,255,255,0,0,0,0,255,0,0,0,255,255,0,0,0,0,255,0,0,0,
  255,255,0,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,255,255,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,255,0,255,255,0,0,0,255,255,255,255,255,0,0,0,
  0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,255,
  0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,
  0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,0,255,255,255,0,255,255,0,0,0,0,255,255,255,0,255,255,0,0,0,0,255,255,255,0,255,255,0,0,0,0,255,255,255,
  0,255,255,0,0,0,0,255,255,255,0,255,255,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,255,255,255,0,0,0,255,0,0,255,
  0,0,0,0,0,255,255,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,0,0,0,0,255,255,255,0,0,0,255,255,255,0,0,0,0,255,0,0,0,255,0,0,
  0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,255,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,
  0,255,255,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,0,0,255,255,255,0,0,0,255,255,255,0,0,255,255,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,0,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,255,0,255,0,0,
  0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,
  0,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,255,0,0,0,0,255,0,0,255,0,0,255,0,0,0,255,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,
  0,0,255,255,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,
  0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,255,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,255,0,0,0,0,0,255,0,255,255,255,0,0,0,0,0,255,0,255,255,255,255,0,255,0,0,0,0,
  0,255,0,255,0,255,0,0,0,0,255,0,0,255,255,255,0,255,0,0,0,0,255,0,255,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,
  0,0,0,255,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,255,0,0,255,0,0,0,0,0,255,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,0,0,255,0,255,0,255,0,0,0,0,
  0,255,0,255,255,255,0,0,0,0,255,0,255,0,0,0,0,255,0,0,0,0,255,0,255,0,0,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,255,255,255,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,0,0,0,0,
  0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,255,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,255,255,0,0,255,0,0,0,0,
  0,255,255,255,0,255,0,0,0,0,255,0,255,0,0,0,0,255,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,
  0,255,255,0,0,255,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,255,0,0,255,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,
  0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,
  0,0,255,0,0,0,0,0,0,0,255,0,0,255,255,255,0,0,0,0,0,255,0,255,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,255,255,255,255,0,255,0,0,0,0,
  255,0,0,255,0,0,0,0,0,0,255,0,0,255,255,255,0,255,0,0,0,0,255,0,0,255,0,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,255,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,
  0,255,255,255,255,255,255,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,255,255,255,255,0,0,0,0,255,0,0,255,0,0,255,0,0,0,0,0,255,0,255,255,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,
  255,0,0,255,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,255,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,
  0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,
  255,255,255,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,0,0,255,255,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,
  0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,
  0,0,0,255,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,255,255,0,0,0,
  0,0,255,255,255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,0,0,0,
  0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,
  255,0,0,255,255,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,
  255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,
  255,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,0,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,0,255,255,
  0,0,0,0,0,0,0,255,255,0,0,0,0,255,0,0,255,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,
  255,0,0,0,255,0,0,0,0,0,255,0,255,255,255,255,0,0,0,255,255,0,0,255,255,0,0,0,0,0,255,0,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,
  0,255,255,255,0,255,255,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,
  0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,255,0,
  0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,
  255,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,255,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,255,255,
  255,255,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,255,0,
  0,0,0,0,255,255,255,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,
  255,0,0,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,
  0,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,255,255,255,0,0,0,255,0,0,0,0,255,0,
  0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
  255,255,255,0,255,255,0,0,0,0,255,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,
  255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,255,255,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,255,0,
  0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,0,0,255,0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,
  255,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,255,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,
  0,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,255,255,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,0,255,0,
  0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,0,255,255,255,0,0,0,0,0,255,255,255,0,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,
  255,0,0,0,0,0,0,0,0,255,255,0,0,255,255,255,0,0,0,0,0,255,255,0,255,255,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,255,
  0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,
  0,0,0,255,0,0,0,255,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,255,255,255,0,0,255,255,0,0,0,0,0,0,
  255,255,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,255,255,255,0,0,0,0,0,0,0,255,255,255,
  0,0,0,0,255,255,255,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,255,0,0,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,255,
  255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,
  0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,255,255,0,0,0,0,0,
  0,255,255,255,0,255,0,0,0,0,0,0,255,255,255,0,0,0,255,255,255,255,255,255,0,0,0,0,0,255,0,255,0,0,0,0,255,255,0,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,
  0,0,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,
  0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,
  0,255,255,255,255,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,255,0,0,0,0,
  255,0,0,0,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,0,0,0,0,0,255,255,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,0,
  0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,255,0,
  0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,
  0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,
  255,0,0,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,
  255,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,255,255,0,0,0,0,255,
  255,255,255,255,0,0,0,0,255,0,0,0,255,0,0,0,255,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,255,255,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,
  0,255,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,0,255,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,
  255,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,
  0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,0,
  0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,255,255,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,
  255,255,255,255,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,255,0,255,0,0,0,0,0,
  0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,255,255,255,255,255,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,
  0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,255,255,
  255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,
  255,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,
  255,0,255,255,255,0,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,
  0,0,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,255,255,255,0,0,0,0,0,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,255,255,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,255,255,255,0,0,0,0,255,255,0,255,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,0,255,
  255,255,0,255,255,255,0,0,0,255,255,0,255,255,0,0,0,0,0,255,255,0,0,255,255,0,0,0,0,0,255,255,255,0,255,0,0,0,255,255,0,255,255,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,0,0,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,255,0,255,255,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,
  255,0,0,0,255,0,0,0,0,0,255,255,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,255,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,255,255,0,0,0,0,255,255,255,0,0,0,0,0,255,0,0,0,0,0,
  0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,
  0,0,0,0,255,255,0,0,0,0,255,255,255,0,255,255,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,255,255,255,255,255,255,255,0,0,0,0,255,0,0,255,0,0,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,
  255,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,255,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,255,0,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,255,255,255,255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,255,0,0,0,0,
  0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,255,0,0,0,
  0,0,0,0,255,255,0,0,0,255,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,
  255,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,255,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,
  0,0,0,0,0,0,0,0,0,255,0,0,0,255,255,255,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,255,0,0,0,255,0,0,0,255,255,0,0,0,0,0,
  255,0,255,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,255,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,0,0,255,255,0,0,
  0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,255,0,0,0,255,255,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,255,0,0,
  0,255,0,0,0,255,255,255,255,255,255,255,0,0,0,0,255,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,255,255,0,0,0,255,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,0,0,
  0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,255,255,255,255,255,255,255,0,0,0,255,255,255,0,255,255,0,255,0,0,0,0,255,255,255,0,255,255,0,0,0,0,
  0,255,0,255,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,255,255,0,255,255,0,0,0,255,0,255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,255,255,0,0,0,0,
  0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,255,0,255,
  0,0,0,0,255,255,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,
  0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,255,255,255,0,0,0,0,255,0,0,255,0,
  0,0,0,255,255,0,0,0,0,0,255,255,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,255,0,0,
  0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,255,255,255,255,
  0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,0,0,0,0,0,0,255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,255,0,0,0,
  0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,255,255,0,0,0,0,0,255,255,0,0,0,0,255,255,0,0,0,0,255,255,0,0,
  0,255,255,0,0,0,0,255,0,0,0,255,255,255,255,255,255,0,0,0,255,0,0,255,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,255,255,255,0,0,0,0,255,0,0,0,0,255,0,255,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,255,255,0,0,0,0,255,0,0,255,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,
  0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,255,0,0,0,0,255,0,0,255,0,0,0,255,0,0,255,0,0,0,0,255,255,0,0,0,255,255,0,0,0,0,255,255,0,0,0,0,255,255,0,0,0,
  0,0,255,0,0,0,0,255,0,0,0,255,255,255,255,255,255,0,0,0,0,0,255,0,0,255,0,0,0,255,0,0,255,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,255,255,0,0,0,255,255,255,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,0,255,0,0,0,0,0,255,255,255,255,255,0,255,255,0,0,0,255,255,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,255,255,0,255,255,0,0,0,255,
  0,0,255,0,0,0,255,255,255,255,0,0,0,255,0,255,0,0,0,0,255,255,255,0,0,0,0,255,0,0,255,0,0,0,0,255,255,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,
  0,255,255,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,
  0,0,255,255,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,255,0,255,255,0,255,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,255,255,0,255,255,0,0,0,0,
  255,255,0,0,0,0,0,0,255,255,0,0,0,0,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,0,255,0,0,0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,255,0,0,0,0,0,255,0,0,
  0,0,255,255,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,255,0,255,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,255,0,0,255,0,0,0,0,0,
  255,255,0,0,0,0,0,0,0,255,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,0,0,0,255,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,
  0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,255,255,255,255,0,255,255,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,255,0,0,0,0,255,
  0,0,255,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,0,0,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,
  0,0,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,0,255,0,255,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,
  0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,255,0,0,0,0,255,0,0,0,255,255,255,255,255,0,0,0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,
  0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,255,0,0,0,0,0,255,255,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,255,0,255,0,0,0,255,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,255,255,255,255,255,255,255,255,0,0,0,255,255,255,255,255,255,255,255,0,0,0,255,255,255,255,255,
  255,255,255,0,0,0,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,0,0,255,0,0,0,255,255,0,255,0,0,0,255,0,0,0,0,0,255,0,0,0,255,255,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,0,0,255,255,0,0,0,0,255,0,255,255,0,0,0,0,255,0,0,0,255,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

}

