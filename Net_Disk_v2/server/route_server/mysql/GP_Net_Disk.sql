-- MySQL dump 10.13  Distrib 8.0.37, for Linux (x86_64)
--
-- Host: localhost    Database: GP_Net_Disk
-- ------------------------------------------------------
-- Server version	8.0.37-0ubuntu0.22.04.3

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `MD5_IP`
--

DROP TABLE IF EXISTS `MD5_IP`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MD5_IP` (
  `ID` int NOT NULL AUTO_INCREMENT,
  `file_md5` varchar(32) DEFAULT NULL,
  `file_server1_ip` varchar(16) DEFAULT NULL,
  `file_server1_port` varchar(5) DEFAULT NULL,
  `file_server2_ip` varchar(16) DEFAULT NULL,
  `file_server2_port` varchar(5) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `file_md5` (`file_md5`),
  CONSTRAINT `MD5_IP_ibfk_1` FOREIGN KEY (`file_md5`) REFERENCES `VFS` (`file_md5`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MD5_IP`
--

LOCK TABLES `MD5_IP` WRITE;
/*!40000 ALTER TABLE `MD5_IP` DISABLE KEYS */;
/*!40000 ALTER TABLE `MD5_IP` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `VFS`
--

DROP TABLE IF EXISTS `VFS`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `VFS` (
  `code` int NOT NULL AUTO_INCREMENT,
  `pre_code` int DEFAULT '-1',
  `file_name` varchar(256) DEFAULT NULL,
  `file_type` char(1) DEFAULT NULL,
  `file_size` int DEFAULT '0',
  `file_md5` varchar(32) DEFAULT '0',
  `is_valid` int DEFAULT '1',
  `owner_uid` int DEFAULT NULL,
  PRIMARY KEY (`code`),
  KEY `owner_uid` (`owner_uid`),
  KEY `idx_file_md5` (`file_md5`),
  CONSTRAINT `VFS_ibfk_1` FOREIGN KEY (`owner_uid`) REFERENCES `user_table` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `VFS`
--

LOCK TABLES `VFS` WRITE;
/*!40000 ALTER TABLE `VFS` DISABLE KEYS */;
/*!40000 ALTER TABLE `VFS` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_table`
--

DROP TABLE IF EXISTS `user_table`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user_table` (
  `user_id` int NOT NULL AUTO_INCREMENT,
  `user_name` varchar(32) DEFAULT NULL,
  `salt` varchar(16) DEFAULT NULL,
  `crypt_passwd` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_table`
--

LOCK TABLES `user_table` WRITE;
/*!40000 ALTER TABLE `user_table` DISABLE KEYS */;
/*!40000 ALTER TABLE `user_table` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-06-20 13:58:35
