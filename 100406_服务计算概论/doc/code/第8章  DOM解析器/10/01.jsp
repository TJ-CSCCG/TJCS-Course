<%@ page import="javax.xml.parsers.*"%>
<%@ page import="org.w3c.dom.*"%>
<%@ page contentType="text/html;charset=GB2312" language="java" %>
<html>
  <head>
      <%
        try{
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse("08.xml");
            Element root = document.getDocumentElement();
            NodeList nodeList = document.getElementsByTagName("车次");
       %>
      <title>
          XML与JSP结合
      </title>
      </head>
  <body>
    <center>
        <h3><%=root.getNodeName()%></h3>
        <table>
            <tr align="center" bgcolor="#AAAAAA">
                <th>车次</th><th>类型</th><th>始发站</th><th>终到站</th>
                <th>运行里程</th><th>始发时间</th><th>终到时间</th><th>运行时间</th>
            </tr>
            <%
            for(int i=0;i<nodeList.getLength();i++){
                Node node = nodeList.item(i);
                NodeList cNodeList = node.getChildNodes();
             %>
            <tr align="center" bgcolor="#DDDDDD">
                <td>
                    <%=cNodeList.item(0).getNodeValue()%>
                 </td>
              <%
                NamedNodeMap map = node.getAttributes();
                Attr attr = (Attr)map.item(0);
               %>
                <td>
                    <%=attr.getValue()%>
                </td>
                <%
                for(int j=1;j<cNodeList.getLength();j++){
                    Node cNode = cNodeList.item(j);
                    if(cNode.getNodeType()==Node.ELEMENT_NODE){
                        Element eNode = (Element)cNode;
                 %>
                 <td>
                   <%=eNode.getTextContent()%>
                 </td>
                 <%
                        if(eNode.hasAttributes()){
                            NamedNodeMap map1 = eNode.getAttributes();
                            Attr attr1 = (Attr)map1.item(0);
                 %>
                 <td>
                     <%=attr1.getValue()%>
                 </td>
                <%
                          }
                      }
                    }
                %>
                  </tr>
                <%
                 }
               }catch(Exception e){
                     e.printStackTrace();
                 }
                %>
        </table>
    </center>
  </body>
</html>