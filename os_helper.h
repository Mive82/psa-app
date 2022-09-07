#ifndef OS_HELPER_H
#define OS_HELPER_H

#include <QObject>

class os_helper : public QObject
{
    Q_OBJECT
public:
    explicit os_helper(QObject *parent = nullptr);
    ~os_helper();

signals:

public slots:

    /**
     * @brief Returns the cpu temperature
     * @return -1 if an error occured, the temperature otherwise.
     */
    int get_cpu_temp();

    /**
     * @brief Returns the lcd brightness in range [0,255]
     * @return -1 if an error occured, the brightness otherwise.
     */
    int get_lcd_brightness();

    /**
     * @brief Set the lcd brightness in range [0,255]
     * @param brightness Brightness to set
     * @return -1 if an error occured, 0 otherwise.
     */
    int set_lcd_brightness(int brightness);

    /**
     * @brief Get the lcd backlight power state
     * @return 0 if backlight is off, 1 if its on, -1 if error
     */
    int get_lcd_backlight();

    /**
     * @brief Turn the lcd backlight on or off
     * @param on - power state, true is on, false is off
     * @return -1 if error, 0 otherwise
     */
    int set_lcd_backlight(bool on);

private:
    int cpu_temp_file;
    int lcd_bright_file;
    int lcd_backlight_file;
};

#endif // OS_HELPER_H
