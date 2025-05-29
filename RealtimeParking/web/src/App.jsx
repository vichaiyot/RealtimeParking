import React, { useState, useEffect } from 'react';
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, set, get } from 'firebase/database';
import { getStorage } from 'firebase/storage';
import './App.css';
import car1Image from './img/car1.png';

function App() {
  const firebaseConfig = {
    apiKey: 'AIzaSyBQjpw-E5-K6y60R3gD97moqdMaIMik',
    authDomain: 'realtime-parking-bf00c.firebaseapp.com',
    databaseURL: 'https://realtime-parking-bf00c-default-rtdb.firebaseio.com',
    projectId: 'realtime-parking-bf00c',
    storageBucket: 'realtime-parking-bf00c.firebasestorage.app',
    messagingSenderId: '310377636017',
    appId: '1:310377636017:web:8091cfbad496b2e4f08290',
    measurementId: 'G-7260PMWZ8L',
  };

  const app = initializeApp(firebaseConfig);
  const database = getDatabase(app);
  const storage = getStorage(app);

  const [sensorStates, setSensorStates] = useState({
    D1: false,
    D2: false,
    D3: false,
    D4: false,
  });

  
  // Fetch sensor states
  useEffect(() => {
    const sensorRef = ref(database, '/Sensors');
    onValue(sensorRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        setSensorStates({
          D1: data.Sensor1 === 'FULL',
          D2: data.Sensor2 === 'FULL',
          D3: data.Sensor3 === 'FULL',
          D4: data.SensorD4 === 'OBJECT_PRESENT',
        });
      }
    });
  }, [database]);

 
  const [showPopup, setShowPopup] = useState(false);
  const [userName, setUserName] = useState('');

  useEffect(() => {
    const cardRef = ref(database, '/CardID');
  
    onValue(cardRef, async (snapshot) => {
      const cardID = snapshot.val();
      console.log("555");
      if (cardID) {
        try {
          // ดึงข้อมูลทั้งหมดจาก '/Users'
          const usersRef = ref(database, '/Users');
          const usersSnapshot = await get(usersRef);
          
          if (usersSnapshot.exists()) {
            const usersData = usersSnapshot.val();
            console.log("666");
            // วนลูปเช็ค cardID ของแต่ละผู้ใช้
            Object.values(usersData).forEach((userData) => {
              if (userData.cardID === cardID) {
                // แสดงpopup
                console.log("333");
                setUserName(userData.firstname);
                setShowPopup(true); // Trigger popup display
              }
            });
          }
        } catch (error) {
          console.error('เกิดข้อผิดพลาดในการดึงข้อมูลผู้ใช้:', error);
        }
      }
    });
  }, [database]);


  // Handle exit button click
  const handleExit = () => {
    const exitRef = ref(database, '/Exit');
    set(exitRef, 'OPEN');
  };

  return (
    <div>
      <header>
        <nav>
          <div className="logo">
            <h1>Realtime Parking</h1>
          </div>
        </nav>
      </header>
      {showPopup && (
        <div className="popmodel">
          <div className="popup">
            <p>{userName} ENTRY</p>
            <button onClick={() => setShowPopup(false)}>Close</button>
          </div>
        </div>
      )}
    
      <section>
        <div className="main-content">
          <div id="park">
            <div className="barall">
              <div className="bar"></div>
              <div className="bar"></div>
              <div className="bar"></div>
              <div className="bar"></div>
            </div>

            {(sensorStates.D1 && sensorStates.D2 && sensorStates.D3) ? (
              <div className="full-message">
                <p>This parking lot is full</p>
                <div className="cary1">
                  <img className="car1-display" src={car1Image} alt="Car 1" />
                </div>
                <div className="cary2">
                  <img className="car2-display" src={car1Image} alt="Car 2" />
                </div>
                <div className="cary3">
                  <img className="car3-display" src={car1Image} alt="Car 3" />
                </div>
              </div>
            ) : (
              <>
                <div className="car1">
                  <img
                    className="car1-display"
                    src={car1Image}
                    alt="Car 1"
                    style={{ display: sensorStates['D1'] ? 'block' : 'none' }}
                  />
                  <h2 className="log1" style={{ display: sensorStates['D1'] ? 'none' : 'block' }}>A1</h2>
                </div>
                <div className="car2">
                  <img
                    className="car2-display"
                    src={car1Image}
                    alt="Car 2"
                    style={{ display: sensorStates['D2'] ? 'block' : 'none' }}
                  />
                  <h2 className="log2" style={{ display: sensorStates['D2'] ? 'none' : 'block' }}>A2</h2>
                </div>
                <div className="car3">
                  <img
                    className="car3-display"
                    src={car1Image}
                    alt="Car 3"
                    style={{ display: sensorStates['D3'] ? 'block' : 'none' }}
                  />
                  <h2 className="log3" style={{ display: sensorStates['D3'] ? 'none' : 'block' }}>A3</h2>
                </div>
              </>
            )}

            <div className="exit">
              <div className="exit-box">
                <button
                  onClick={handleExit}
                  disabled={!sensorStates.D4}
                  className={`btn ${sensorStates.D4 ? 'enabled' : 'disabled'}`}
                >
                  EXIT
                </button>
              </div>
            </div>
          </div>
        </div>

        
      </section>
    </div>
  );
}

export default App;
