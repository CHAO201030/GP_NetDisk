-- MySQL dump 10.13  Distrib 8.0.37, for Linux (x86_64)
--
-- Host: localhost    Database: Net_Disk
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
-- Table structure for table `VFS`
--

DROP TABLE IF EXISTS `VFS`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `VFS` (
  `code` int NOT NULL AUTO_INCREMENT,
  `pre_code` int DEFAULT '-1',
  `file_name` varchar(255) DEFAULT NULL,
  `file_type` char(1) DEFAULT NULL,
  `file_size` int DEFAULT NULL,
  `file_md5` varchar(32) DEFAULT NULL,
  `file_count` int DEFAULT '1',
  `owner_uid` int DEFAULT NULL,
  `cluster_ip_1` varchar(16) DEFAULT NULL,
  `cluster_port_1` varchar(5) DEFAULT NULL,
  `cluster_ip_2` varchar(16) DEFAULT NULL,
  `cluster_port_2` varchar(5) DEFAULT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `file_size_1` int DEFAULT NULL,
  `file_size_2` int DEFAULT NULL,
  PRIMARY KEY (`code`),
  KEY `fk_1` (`owner_uid`),
  CONSTRAINT `fk_1` FOREIGN KEY (`owner_uid`) REFERENCES `user_table` (`user_id`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `VFS`
--

LOCK TABLES `VFS` WRITE;
/*!40000 ALTER TABLE `VFS` DISABLE KEYS */;
INSERT INTO `VFS` VALUES (1,-1,'dir1','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-10 05:50:26',0,0),(2,1,'dir2','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-10 05:56:36',0,0),(3,1,'dir3','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-10 06:05:25',0,0),(4,2,'dir4','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-10 10:55:00',0,0),(5,3,'dir1','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-10 13:31:37',0,0),(6,-1,'test1','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-11 03:24:38',0,0),(7,1,'test2','d',0,'0',1,1,NULL,NULL,NULL,NULL,'2024-06-11 03:26:59',0,0),(8,-1,'root_test','d',0,'0',1,6,NULL,NULL,NULL,NULL,'2024-06-12 05:49:10',0,0),(9,-1,'123','f',123,'qdqd',1,1,NULL,NULL,NULL,NULL,'2024-06-12 06:13:20',0,0),(10,5,'file1','f',123,'qweqwe',1,1,NULL,NULL,NULL,NULL,'2024-06-12 08:29:54',0,0),(11,3,'file2','f',123,'qweqwe',1,1,NULL,NULL,NULL,NULL,'2024-06-12 08:30:43',0,0);
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
  `salt` varchar(12) DEFAULT NULL,
  `crypt_passwd` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_table`
--

LOCK TABLES `user_table` WRITE;
/*!40000 ALTER TABLE `user_table` DISABLE KEYS */;
INSERT INTO `user_table` VALUES (1,'xixi','$6$rCQwDHKk$','pCbmlzZLE7lsdv37ouexwx/IJu5Eu4roweHp/sowt/3NTR3PrDf80.R/rPMQ33ggnhFbMXcCE1zVi1npg./xR.'),(6,'root','$6$GeoFAnIc$','rmK73Jl9rb1s3f/rVi/pqPinr.tQU7b.urAU4t0oRtIiTx3UzXBmeARP3xpfnDcTuGDZGdp86mL7HlAaEwRgn.'),(7,'galaxy','123','123');
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

-- Dump completed on 2024-06-13 16:32:55
