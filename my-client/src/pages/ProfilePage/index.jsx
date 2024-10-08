import { Link } from "react-router-dom";
import { ROUTER } from "../../routes/router";
import { useContext } from "react";
import { AppContext } from "../../data/AppContext";
import "./style.scss";

const ProfilePage = () => {
  const { setCurrentPage } = useContext(AppContext);
  return (
    <>
      <div className="main-header">
        <h1> My Profile</h1>

        <ul className="breadcrumb">
          <li>
            <Link
              to={ROUTER.USER.HOME}
              className="active"
              onClick={() => setCurrentPage(ROUTER.USER.HOME)}
            >
              Dashboard
            </Link>
          </li>
          /
          <li>
            <Link
              to={ROUTER.USER.STATICS}
              onClick={() => setCurrentPage(ROUTER.USER.STATICS)}
            >
              Data Sensor
            </Link>
          </li>
          /
          <li>
            <Link
              to={ROUTER.USER.HOME}
              onClick={() => setCurrentPage(ROUTER.USER.HISTORY)}
              className="active"
            >
              History
            </Link>
          </li>
        </ul>
      </div>
      <div className="my-inf">
        <ul>
          <li>
            Họ tên: <span>Nguyễn Quang Minh</span>{" "}
          </li>
          <li>
            Mã Sinh Viên: <span>B21DCCN531</span>
          </li>
          <li>
            Lớp: <span>D21CNPM02</span>
          </li>
          <li>
            File báo cáo: <a href="https://github.com/Minh1509/IotWeather.git" style = {{color: "#1976d2"}}>BaoCao.pdf</a>
          </li>
          <li>
            Source code:  <a href="https://github.com/Minh1509/IotWeather.git" style={{color: "#1976d2"}}>IotWeather.git</a>
          </li>
          <li>
            API Docs: <a href="https://github.com/Minh1509/IotWeather/blob/main/api_docs.json" style = {{color: "#1976d2"}}>Api_docs.json</a>
          </li>
        </ul>
        <img src="/images/myimage.jpg" alt="MyImage" />
      </div>
    </>
  );
};
export default ProfilePage;
