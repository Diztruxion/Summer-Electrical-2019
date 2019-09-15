<?php

try {
	// use request (merger of get, post, cookie) 
	if (empty($_REQUEST['action'])) {
		throw new InvalidArgumentException('action must be specified');
	}
	
	$action = $_REQUEST['action'];
	switch ((string)$action) {
		case 'upload_waypoints':
			// accept a file (waypoint_file) and put it in $destinationFile
			$destinationFile = '/projects/waypointlist.txt';
			if (!empty($_FILES['waypoint_file'])) {
				 // Undefined | Multiple Files | $_FILES Corruption Attack
				// If this request falls under any of them, treat it invalid.
				if (!isset($_FILES['waypoint_file']['error']) || is_array($_FILES['waypoint_file']['error']) ) {
				    throw new RuntimeException('Invalid parameters.');
				}

				switch ($_FILES['waypoint_file']['error']) {
				    case UPLOAD_ERR_OK:
					break;
				    case UPLOAD_ERR_NO_FILE:
					throw new RuntimeException('No file sent.');
				    case UPLOAD_ERR_INI_SIZE:
				    case UPLOAD_ERR_FORM_SIZE:
					throw new RuntimeException('Exceeded filesize limit.');
				    default:
					throw new RuntimeException('Unknown errors.');
				}

				if (!move_uploaded_file($_FILES['waypoint_file']['tmp_name'], $destinationFile)) {
					throw new RuntimeException("error uploading waypoint file '{$destinationFile}', check www-data has permission");
				}
				$message = "{$destinationFile} saved";
			}
			break;
		case 'picture':
			// expect argument 'time'
			// take a picture at the specified location
			if (empty($_REQUEST['time'])) {
				throw new InvalidArgumentException('action time must be specified');
			}
			$time = (int)$_REQUEST['time'];
			// passing a string in here would introduce a command injection vuln
			//$command = "sudo python /home/pi/takepiccont.py {$time} 2>&1";
			$command = "sudo python /home/pi/takepic.py {$time} 2>&1";
			$returnCode = null;
			$output = null;
			exec($command, $output, $returnCode);
			// handle return code
			if ($returnCode !== 0) {
				// error
				$output = implode("\n", $output);
				throw new RuntimeException("return code wasnt 0 from '{$command}'. output:\n{$output}");
			}
			$message = $output;
			break;
		case 'check_cameras':
			// call picture function at waypoint -1 once on each camera to make sure its working.
			// this can be made faster by exec'ing curl to take the picture, ignoring the response, and checking if the pictures are created client-side
			$ips = [
				'192.168.137.101',
				'192.168.137.103',
				'192.168.137.105',
			    
			//	'192.168.137.102',
			//	'192.168.137.104',
			//	'192.168.137.106',
			];
			
			foreach ($ips as $ip) {
				$url = "http://{$ip}/?action=picture&waypoint=-1";
				$response = file_get_contents($url);
				if (!$json = json_decode($response)) {
					throw new RuntimeException("camera at ip {$ip} taking picture failed: ".$response);
				}
				if (!$json->success) {
					throw new RuntimeException("json failed to parse for camera at ip {$ip} when taking picture: ".$response);
				}
			}
			$message = 'success';
			break;
		case 'start_scan':
			$cmd = 'sudo /projects/run_main.sh';
			$returnCode = null;
			
			$output = null;
			exec($cmd, $output, $returnCode);
			if (0 !== $returnCode) {
				throw new RuntimeException("'{$cmd}' return code was not 0 (was '{$returnCode}')");
			}
			$message = 'success';
			break;
		case 'emergency_stop':
			$cmd = 'sudo killall main';
			exec($cmd);
			$message = $cmd;
			break;
		case 'shutdown':
			$cmd = 'sudo shutdown -h now';
			exec($cmd);
			$message = $cmd;
			break;
		case 'shutdown_all':
			// consolidate these somewhere, or possibly split these into separate functions/files
			$ips = [
				'192.168.137.101',
				'192.168.137.103',
				'192.168.137.105',
				'192.168.137.102',
				'192.168.137.104',
				'192.168.137.106',
			];
			foreach ($ips as $ip) {
				$url = "http://{$ip}/?action=shutdown";
				$response = file_get_contents($url);
				if (!$json = json_decode($response)) {
					//throw new RuntimeException("camera at ip {$ip} shutdown failed: ".$response);
				}
				if (!$json->success) {
				//	throw new RuntimeException("json failed to parse for camera at ip {$ip} when shutting down: ".$response);
				}
			}
			$cmd = 'sudo shutdown -h now';
			exec($cmd);
			$message = $cmd;
			break;
		default:
			throw new InvalidArgumentException("unknown action '{$action}' specified");
	}
	$response = [
		'success' => true,
		'response' => $message
	];
}
catch (Exception $ex) {
	$response = [
		'success' => false,
		'error' => print_r($ex, true)
	];
}
echo json_encode($response, JSON_PRETTY_PRINT);
