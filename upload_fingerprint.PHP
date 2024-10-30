<?php
// Content-type header নিশ্চিত করুন
header("Content-Type: application/json");

// ডাটাবেস সংযোগের তথ্য
$servername = "localhost"; // সাধারণত localhost হবে
$username = "root"; // আপনার ডাটাবেজ ইউজারনেম
$password = ""; // আপনার ডাটাবেজ পাসওয়ার্ড
$dbname = "fingerprintdb"; // ডাটাবেজের নাম

// ডাটাবেজে সংযোগ করা
$conn = new mysqli($servername, $username, $password, $dbname);

// সংযোগ চেক করা
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// POST রিকোয়েস্ট চেক করা
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // JSON ইনপুট গ্রহণ করা
    $input = file_get_contents("php://input");
    $data = json_decode($input, true);

    // ডেটা যাচাই করা
    if (isset($data['id'])) {
        $fingerprintId = $data['id'];

        // ডাটাবেজে ডেটা ইনসার্ট করা
        $stmt = $conn->prepare("INSERT INTO fingerprints (fingerprint_id) VALUES (?)");
        $stmt->bind_param("i", $fingerprintId);

        if ($stmt->execute()) {
            // সফল ইনসার্ট হলে সাড়া প্রদান করা
            $response = [
                "status" => "success",
                "message" => "Fingerprint ID stored successfully",
                "fingerprint_id" => $fingerprintId
            ];
        } else {
            // ত্রুটি হলে সাড়া প্রদান করা
            $response = [
                "status" => "error",
                "message" => "Failed to store fingerprint ID"
            ];
        }
        $stmt->close();
    } else {
        // যদি `id` না পাওয়া যায়
        $response = [
            "status" => "error",
            "message" => "No fingerprint ID found in request"
        ];
    }
} else {
    // যদি POST রিকোয়েস্ট না হয়
    $response = [
        "status" => "error",
        "message" => "Invalid request method"
    ];
}

// JSON ফরম্যাটে সাড়া প্রদান করা
echo json_encode($response);

// ডাটাবেজ সংযোগ বন্ধ করা
$conn->close();
?>
