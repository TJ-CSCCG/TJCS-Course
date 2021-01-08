<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
  <xsl:template math="/">
	<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td align="center">nmae</td>
		</tr>
		<xsl:for-each select="/message/persion/name">
		<tr>
			<td align="center"><xsl:value-of select="." /></td>
		</tr>
		</xsl:for-each>
		</table>
		</center>
	</body>
	</html>
  </xsl:template>
</xsl:stylesheet>