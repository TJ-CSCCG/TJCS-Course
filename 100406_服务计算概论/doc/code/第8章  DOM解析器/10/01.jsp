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
            NodeList nodeList = document.getElementsByTagName("����");
       %>
      <title>
          XML��JSP���
      </title>
      </head>
  <body>
    <center>
        <h3><%=root.getNodeName()%></h3>
        <table>
            <tr align="center" bgcolor="#AAAAAA">
                <th>����</th><th>����</th><th>ʼ��վ</th><th>�յ�վ</th>
                <th>�������</th><th>ʼ��ʱ��</th><th>�յ�ʱ��</th><th>����ʱ��</th>
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